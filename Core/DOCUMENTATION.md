# 3D Printer Firmware — Core Documentation

## 1. Project Overview

This is the **microcontroller-side firmware** of a 3D printer built around an
**STM32F401** MCU. The architecture is inspired by **Klipper**:

- **Motion planning runs on the PC (host).**
- The MCU acts as a *"dumb executor"* — it only plays back pre-planned motion
  commands and performs low-level control.
- The two sides communicate over **USB** (USB CDC virtual serial port).

The code in this folder (`Core/`) implements the application logic: stepper
motor generation, buffering and parsing of commands received over USB, the main
state machine, endstop / emergency-stop handling and heater (PID) control.

> **Status:** The project is **not finished**. Sections marked with
> `[TODO]`/plain placeholders indicate work that still needs to be completed.

---

## 2. Software Architecture

```
                   USB (CDC virtual COM)
                            |
                     +------v-------+
                     | usb_praser   |  ISR context: splits the USB byte stream,
                     | (framing)    |  copies payloads to per-axis buffers
                     +------+-------+
                            |  1 = X , 2 = Y , 3 = Z , 4 = E , else "misc"
              +-------------+----------------+-----------------+
              v                              v                 v
        per-axis buffers                 usb_circ_buf        header_circ_buf
        (x/y/z/e_buffer)                    |                    |
              |                        +----v----+          +----v-------+
              |                        | protocol_praser     | (frame     |
              |                        | replies/TX)         |  dispatch) |
              |                        +---------+           +------------+
              v                               ^
        +----------------------------------------------+
        |                    axis.c                    |
        |  axis_map[axis] -> {motor, buffer, channel}  |
        +----------------------------------------------+
              |
              v  HAL_TIM_PWM_PulseFinishedCallback
        +----------------------------------------------+
        |                stepper_motor.c               |
        |  check_next_pulse() -> PWM duty 0% / 50%     |
        |  set_motor_velocity_and_dir()                |
        +----------------------------------------------+
```

The actual code in the ISR (`HAL_TIM_PWM_PulseFinishedCallback`) drives one
stepper per PWM channel of **TIM3**, in sync because all channels share one
timer (and thus one clock).

### Module map

| Module | Files | Responsibility |
|---|---|---|
| `axis` | `Src/axis.c`, `Inc/axis.h` | Axis mapping (X/Y/Z/E) → motor, buffer, timer channel |
| `stepper_motor` | `Src/stepper_motor.c`, `Inc/stepper_motor.h` | Stepper abstraction, PWM pulse generation, homing |
| `t_velocity` | `Src/t_velocity.c`, `Inc/t_velocity.h` | Packed velocity command (direction, steps, period) |
| `circular_buffer` | `Src/circular_buffer.c`, `Inc/circular_buffer.h` | Ring buffer with block copy and partial-object handling |
| `usb_praser` | `Src/usb_praser.c`, `Inc/usb_praser.h` | USB stream framing/routing (ISR context) |
| `protocol_praser` | `Src/protocol_praser.c`, `Inc/protocol_praser.h` | Frame dispatch + response/TX (main-loop context) |
| `remote_state` | `Src/remote_state.c`, `Inc/remote_state.h` | Remote (host) state mirrors sent in status frames |
| `emergency_stop` | `Src/emergency_stop.c`, `Inc/emergency_stop.h` | Emergency stop of steppers and heaters |
| `pid` | `Src/pid.c`, `Inc/pid.h` | PID controller for bed and hotend heaters |
| `system_check` | `Src/system_check.c`, `Inc/system_check.h` | Startup self-test (currently placeholder) |

---

## 3. Main State Machine (`main.c`)

The firmware runs as a state machine. `currentState` is a global
`SystemState_t` variable.

| State | Meaning | Exit condition |
|---|---|---|
| `STATE_INIT` | Initialization. Runs `System_Check()` to validate subsystems | On OK → start homing on all axes → `STATE_HOMING`; otherwise → `STATE_STOP` |
| `STATE_HOMING` | Performs homing/referencing (move toward endstops) | When all axis motors have `is_homing == 0` → `STATE_RUN` |
| `STATE_RUN` | Normal printer operation: handles communication and motion | Emergency stop / critical error → `STATE_STOP` |
| `STATE_STOP` | Emergency stop. Steppers and heaters are halted | After `EMERGENCY_RESET` pin is pressed → back to `STATE_RUN` |

> **Note (spec).** The transition to `STATE_STOP` is intentionally done *inside
> an interrupt* for safety (emergency stop EXTI handler, `HAL_GPIO_EXTI_Callback`
> in `main.c`).

The main loop calls `System_Check()`, `usb_tx_process()`, `parse_frame()` and
periodically restarts the ADC-DMA conversion while it dispatches the state
machine. A short `HAL_Delay(5)` paces the loop.

---

## 4. Stepper Motor Control

### 4.1 PWM principle

The PWM base frequency of a timer is:

```
f = ftim / ((PSC + 1) * (ARR + 1))
```

where `PSC` is the prescaler and `ARR` the auto-reload (period) value.
The duty cycle is:

```
CCR / (ARR + 1)
```

The compare register is changed with `__HAL_TIM_SET_COMPARE()` (in
`stepper_motor.c`).

Each stepper motor is wired to its **own PWM channel**, but all channels share
one timer (`TIM3`), which simplifies synchronization — consecutive steps happen
at the same instant across axes.

### 4.2 Speed control via pulse skipping

Rotational speed is controlled by skipping pulses. For every
`HAL_TIM_PWM_PulseFinishedCallback` the duty is set either to **0 %** or
**50 %**:

- If the motor should not emit a step on this tick → duty = 0 %.
- If it should → duty = 50 %.

In this way the pulse frequency (and therefore the rotational speed) can differ
between channels even though the PWM base frequency is shared.

### 4.3 The stepper motor structure (`stepper_motor_t`)

| Field | Description |
|---|---|
| `tim_handler` | Timer handle used for the PWM channel |
| `tim_channel` | Timer compare channel |
| `dir_port` / `dir_pin` | GPIO used to control rotation direction |
| `step_counter` | Remaining steps to execute for the current velocity segment |
| `tick_counter` | Counts timer `PulseFinishedCallback` calls |
| `step_period` | Number of ticks after which `step_counter` is decremented by one (larger value ⇒ slower motor) |
| `pulse` | Stored compare value used for the 50 % pulse |
| `homing_step_period` | Step period used during homing (slow speed) |
| `is_homing` | Flag: 1 while homing, 0 otherwise |

### 4.4 Pulse logic — `check_next_pulse()`

Called from `process_axis()` on every PWM pulse-finish interrupt:

- If `tick_counter == step_period` → set duty to 50 % (a real step), reset
  `tick_counter`, decrement `step_counter`.
- Otherwise → set duty to 0 % and increment `tick_counter`.

When `step_counter` reaches `0`, the axis pulls the next velocity segment
(`t_velocity`) from its buffer and applies it with
`set_motor_velocity_and_dir()`.

### 4.5 Homing

- `start_homing()` sets the direction toward the endstop, an effectively
  infinite step count (`HOMING_INFINITE_STEPS`) and marks `is_homing = 1`.
- `stop_homing()` zeroes the duty, counters and clears `is_homing`.
- Endstop detection is handled in `HAL_GPIO_EXTI_Callback()` (see section 9).

---

## 5. Velocity Command — `t_velocity`

A single planned move segment is packed into a **32-bit value**:

```
31    24 23    16 15       14         2 1 0
|  period[31:16]  |  (unused)  |  steps[13:2]  |  dir[1:0]  |
```

| bits | field | accessor |
|---|---|---|
| `0..1` | direction (`DIR_STOP=0`, `DIR_LEFT=1`, `DIR_RIGHT=2`) | `vel_get_dir()` |
| `2..13` | number of steps in the segment | `vel_get_step_number()` |
| `16..31` | period (ticks between steps) | `vel_get_period()` |

These segments are produced on the PC and pushed, in order, into the matching
per-axis buffer.

---

## 6. Buffering (Circular Buffer)

A custom ring buffer (`circular_buffer.c`) adds two requirements needed at the
USB interface:

1. **Copy whole contiguous memory blocks at once** — required for performance;
   pushing element-by-element inside an interrupt would be too slow.
2. **Accept partial objects** — the USB stream may deliver only a *fragment* of
   a `t_velocity` segment in one interrupt; the leftover must be kept and
   completed on the next chunk.

Key API:

| Function | Purpose |
|---|---|
| `circ_buf_init()` | Initialize buffer (data backing, max length, element size) |
| `circ_buf_push()` / `circ_buf_pop()` | Push/pop a single element (of `elem_size`) |
| `circ_buf_push_many()` | Copy a contiguous block of elements |
| `circ_buf_push_many_uint8()` | Push raw bytes, assembling full elements and buffering the rest (`leftover`) |
| `circ_buf_reset()` / `empty()` / `full()` / `capacity()` / `size()` | State helpers |

---

## 7. USB Receive Path — `usb_praser` (interrupt context)

`read_usb_praser(Buf, Len)` parses the incoming USB byte stream and routes
payload bytes to the appropriate circular buffer:

- A frame starts with a **4-byte header**.
- `header[0]` selects the destination buffer:
  - `1` → X-axis buffer
  - `2` → Y-axis buffer
  - `3` → Z-axis buffer
  - `4` → E (extruder) buffer
  - otherwise → generic `usb_circ_buf` (aux/config data)
- `header[1]` carries the payload size.
- While the header is being assembled it is pushed to `header_circ_buf` for
  later dispatch in the main loop.

> **TODO / unfinished:** `read_usb_praser()` is implemented but **not yet
> connected** to the USB CDC receive callback (which lives outside `Core`, in
> the `USB_DEVICE` folder). Hook the MCU-side receive call into
> `read_usb_praser()` here:

```c
// TODO: call read_usb_praser(Buf, Len) from the CDC "data received" callback.
```

---

## 8. Frame Protocol — `protocol_praser` (main-loop context)

`parse_frame()` reads the header from `header_circ_buf`, then the payload from
`usb_circ_buf`, and dispatches by frame type.

The connection is established via the System_Check() function in the STATE_INIT state and takes the form of a 3-way handshake:

(FRAME_SYNC)        3d Printer -> HOST
(FRAME_SYNC_ACK)    3d Printer <- HOST
(FRAME_ACK)         3d Printer <- HOST

This is followed by the transmission of subsequent packets containing the planned steps.

### 8.1 Frame structure

```c
typedef struct {
    FrameType_e type;        // frame class (header)
    uint8_t    payload_len;  // payload size in bytes
    uint16_t   seq;          // chronological frame sequence number
    uint8_t    payload[MAX_PAYLOAD_SIZE];  // MAX_PAYLOAD_SIZE = 12
} Frame_t;
```

`seq` is the chronological number assigned by the host. The MCU does not execute
a `seq + 1` frame until all commands of frame `seq` have been executed.

### 8.2 Frame types

| Constant | Value | Description / handling |
|---|---|---|
| `FRAME_SYNC` | `0x01` | Time synchronization. Host sends a timestamp (4 bytes). MCU replies with an empty `FRAME_SYNC`. |
| `FRAME_STATUS_REQ` | `0x02` | Host requests status → MCU answers with `FRAME_STATUS_RESP`. |
| `FRAME_STATUS_RESP` | `0x03` | Status frame received from host, updates `remote_state` (`state`, `errors`, `queue_free`). |
| `FRAME_ESTOP` | `0x04` | Emergency stop → `Emergency_Stop_Activate()`. |
| `FRAME_MOVE_X` | `0x05` | X-axis motion command. |
| `FRAME_MOVE_Y` | `0x06` | Y-axis motion command. |
| `FRAME_MOVE_Z` | `0x07` | Z-axis motion command. |
| `FRAME_MOVE_E` | `0x08` | Extruder motion command. |

### 8.3 Frame handlers

- `parse_sync()` — reads the 4-byte host timestamp.
  > **TODO:** actually update the MCU time reference (currently only echoes a
  > `FRAME_SYNC` reply).
- `parse_status_req()` → `send_status()` (+ internal `send_status_req()` helper,
  see note below).
- `parse_status_resp()` — mirrors host state into `remote_state`.
- `parse_estop()` → `Emergency_Stop_Activate()`.
- `parse_frame()` — currently **discards** the payload of MOVE frames
  (`FRAME_MOVE_X..E`); real motion data is routed straight into per-axis buffers
  by `usb_praser`.

### 8.4 Transmit path

- Frames to send are queued into `tx_buffer` via `send_frame()`.
- `usb_tx_process()` drains the queue and transmits 64-byte chunks with
  `CDC_Transmit_FS()`, pushing the chunk back on `USBD_BUSY`.

> **Note / possible refactor:** `send_status_req()` currently sends a
> `FRAME_STATUS_RESP` (not `STATUS_REQ`) — verify the intended framing here.
> A `send_estop()`/ack reply is also commented out.

---

## 9. Emergency Stop

`emergency_stop.c`:

- `Stepper_Emergency_Stop()` — stops PWM interrupt on all TIM3 channels and zeroes
  the compare registers.
- `Heaters_Emergency_Stop()` — zeroes TIM4 PWM and stops heater channels.
- `Emergency_Stop_Activate()` — calls the above and sets `currentState = STATE_STOP`.

Hardware E-stop is monitored on `EMERGENCY_STOP_IN_Pin` (EXTI falling edge) and
the reset on `EMERGENCY_RESET_Pin` (see `main.c` / `stm32f4xx_it.c`).

> **TODO / unfinished:** `Outputs_Disable()` is empty, and some stop paths reuse
> the `PID_Reset` calls commented out in `Heaters_Emergency_Stop()`.

---

## 10. Heater Control (PID)

- **ADC1** reads two channels (bed + hotend temperature sensors) via DMA.
- `HAL_ADC_ConvCpltCallback()` accumulates 16 samples, then runs
  `PID_Compute()` and scales the result with `PID_to_PWM()` onto **TIM4**
  channels 1 and 2 (heaters).
- PID gains are initialized in `main()` and currently have placeholder values.

> **TODO:** set proper PID gains, define target temperatures (`setpoint`) and
> real input scaling for the temperature sensors.

---

## 11. Peripheral Configuration

| Peripheral | Usage | Key settings |
|---|---|---|
| TIM3 | 4 PWM channels for stepper X/Y/Z/E | PSC = 49, ARR = 99 → **≈ 16.8 kHz** base |
| TIM4 | PWM channels 1–2 for heaters | PSC = 167, ARR = 999 → **500 Hz** |
| ADC1 | 2 channels (temperature), DMA | 12-bit, DMA2 stream 0 |
| GPIO EXTI | Endstops X/Y/Z + emergency stop input | Rising edges (endstops), falling edge (E-stop) |
| USB OTG FS | CDC virtual serial port | — |

---

## 12. Known Limitations / To Do

- `System_Check()` is a placeholder — add real tests for communication,
  endstops and peripherals.
- `read_usb_praser()` not yet wired to the USB CDC receive callback.
- MOVE-frame payloads are not processed in `parse_frame()` (only routed/buffered).
- Routing codes in `usb_praser` (`1..4`) and `FrameType_e` MOVE values
  (`0x05..0x08`) are inconsistent — pick one scheme and align both sides.
- `parse_sync()` does not yet update the MCU time.
- PID gains and setpoints still placeholder.
- Emergency-stop path is not fully finalized (`Outputs_Disable()`, comments).

---

## 13. Placeholders

Sections below are intentionally left empty and should be filled in once the
corresponding functionality is completed:

- Detailed USB CDC integration notes (callback binding to `read_usb_praser`).
- Step-rate / feed-rate calibration data.
- Heater tuning results.
- Wiring / pinout table.

---

## 14. Debugging and Error Handling

### 14.1 Debug layer

Two new files provide the diagnostics layer:

| File | Responsibility |
|---|---|
| [`Inc/debug.h`](Inc/debug.h:1) | Build switch, log function declarations, `ASSERT` expression, `UNIT_TEST` stubs |
| [`Src/debug.c`](Src/debug.c:1) | Variadic log functions, guarded ITM/SWO output, safe `__io_putchar` |

Design rules honoured:

- Logging uses variadic **functions**, not variadic macros, in line with
  [`gudeline.txt`](Src/.clinerules/gudeline.txt:52).
- No `goto` is used.
- Release builds define neither `DEBUG` nor `UNIT_TEST`; the log functions
  return immediately and ITM code is compiled out.
- `UNIT_TEST` builds include no HAL headers from [`Inc/debug.h`](Inc/debug.h:1);
  inline no-op stubs keep the test translation units linkable.

Available calls:

```c
debug_error("...", ...);   /* [E] */
debug_warn("...", ...);    /* [W] */
debug_info("...", ...);    /* [I] */
debug_debug("...", ...);   /* [D] */
ASSERT(condition);         /* reports file and line, does not halt */
```

### 14.2 Safe output

[`Src/debug.c`](Src/debug.c:1) checks `CoreDebug->DHCSR`, `ITM->TCR` and
`ITM->TER` before every write. If no debugger is attached, or SWV is disabled,
the data is dropped. The ITM busy wait has a fixed timeout, so a detached
debugger cannot hang the firmware.

A strong `__io_putchar` is provided in [`Src/debug.c`](Src/debug.c:220), so the
weak `_write` already present in [`Src/syscalls.c`](Src/syscalls.c:80) becomes
safe without editing the generated file.

### 14.3 Fault diagnostics

The handlers in [`Src/stm32f4xx_it.c`](Src/stm32f4xx_it.c:86) now select MSP or
PSP using the `EXC_RETURN` bit, then log:

- `PC`, `LR`, `xPSR`
- `R0`, `R1`, `R2`, `R3`, `R12`
- `CFSR`, `HFSR`, `MMFAR`, `BFAR`

The handlers still halt after dumping. [`Error_Handler()`](Src/main.c:616) logs
a fatal message before halting.

### 14.4 Return-code discipline

HAL results are now checked at these points:

- `HAL_Init` and all timer starts in [`Src/main.c`](Src/main.c:164)
- Periodic `HAL_ADC_Start_DMA`, reported once per error episode
- All stop/start calls in [`Src/emergency_stop.c`](Src/emergency_stop.c:5)
- `CDC_Transmit_FS` results in [`Src/protocol_praser.c`](Src/protocol_praser.c:18)

Invalid arguments are rejected with a log and a safe return in
[`Src/stepper_motor.c`](Src/stepper_motor.c:14), [`Src/axis.c`](Src/axis.c:38),
[`Src/circ_buf.c`](Src/circ_buf.c:15), [`Src/usb_praser.c`](Src/usb_praser.c:22),
[`Src/protocol_praser.c`](Src/protocol_praser.c:32) and
[`Src/pid.c`](Src/pid.c:19).

### 14.5 Required build configuration

The project files are outside `Core/`, so these settings must be applied
manually in the IDE build configuration:

| Build config | Preprocessor define | Optimization | Debug level |
|---|---|---|---|
| Debug | `DEBUG` | `-O0` | `-g3` |
| Release | *(none)* | `-Os` | `-g0` |

Also adjust the linker flags as needed:

    -specs=nano.specs
    -u _printf_float   # only if %f is required

Enable SWV in the debug configuration and turn on ITM stimulus port 0.
The `SWV ITM Data Console` must be started, otherwise no log output appears.

### 14.6 Verification status

Only a manual syntax review was performed. The code was **not compiled** and
the unit tests were **not executed**, as requested.

---

## 15. Known Issues (error-path defects left untouched)

The following defects were found while preparing the code for debugging. They
are documented here instead of being fixed, to keep the existing code logic
unchanged.

1. [`Src/system_check.c`](Src/system_check.c:37) uses `error_code |= (~0x02)`,
   which sets almost every bit instead of clearing the timeout bit. The result
   is `0xFF` even after a successful handshake.
2. [`Src/protocol_praser.c`](Src/protocol_praser.c:24) checks
   `circ_buf_capacity()` instead of `circ_buf_avail()`. A queued frame can be
   silently truncated while the function still reports success.
3. [`Src/main.c`](Src/main.c:107) uses `ADC_SUM[0]` for both the bed and the
   print-head PID inputs, and never clears the accumulator after processing.
4. [`Src/usb_praser.c`](Src/usb_praser.c:36) treats `header[1]` as the total
   frame size while `read_frame_bytes` already includes the 4-byte header.
   Frames whose payload is shorter than the header can underflow the unsigned
   subtraction. The new guard detects and resets, but the framing scheme
   itself is unchanged.
5. [`Inc/system_check.h`](Inc/system_check.h:10) declared `getErrorCode()` while
   the implementation was named `get_error_code()`. A compatibility wrapper was
   added in [`Src/system_check.c`](Src/system_check.c:52); the original function
   is unchanged.
6. [`Src/protocol_praser.c`](Src/protocol_praser.c:106) accepted a
   `payload_len` of up to 255 into a 12-byte payload array. Malformed frames are
   now clamped while the byte stream is still drained, but the protocol limit is
   still not enforced on the host side.
7. [`Src/syscalls.c`](Src/syscalls.c:80) still contains the standard weak
   `_write`. It is safe only because [`Src/debug.c`](Src/debug.c:220) supplies a
   guarded strong `__io_putchar`.
8. [`Src/main.c`](Src/main.c:222) calls `axis_init()` and starts all timers
   before `SystemClock_Config()` and the `MX_TIM3_Init()` / `MX_TIM4_Init()`
   calls. At that point the timer handles are zero-initialized. This ordering
   is pre-existing and was not changed; it is likely to fault or make every
   timer-start check fail. The new diagnostics will log the failure, but the
   order must be corrected in a separate change.
9. [`Src/main.c`](Src/main.c:223) initializes `header_circ_buf` with
   `AXIS_BUFFER_SIZE` (1024) although `header_circ_buf_mem` is only
   `HEADER_BUFFER_SIZE` (512) bytes. The circular buffer therefore advertises
   twice its real storage. This is pre-existing and was not changed; use
   `HEADER_BUFFER_SIZE` here in a separate fix.