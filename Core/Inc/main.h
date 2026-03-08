/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdint.h>
#include "circular_buffer.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define EMERGENCY_RESET_Pin GPIO_PIN_13
#define EMERGENCY_RESET_GPIO_Port GPIOC
#define ADDITIONAL_INPUT_Pin GPIO_PIN_2
#define ADDITIONAL_INPUT_GPIO_Port GPIOA
#define ENDSTOP_Z_Pin GPIO_PIN_3
#define ENDSTOP_Z_GPIO_Port GPIOA
#define ENDSTOP_Z_EXTI_IRQn EXTI3_IRQn
#define ENDSTOP_X_Pin GPIO_PIN_4
#define ENDSTOP_X_GPIO_Port GPIOA
#define ENDSTOP_X_EXTI_IRQn EXTI4_IRQn
#define ENDSTOP_Y_Pin GPIO_PIN_5
#define ENDSTOP_Y_GPIO_Port GPIOA
#define ENDSTOP_Y_EXTI_IRQn EXTI9_5_IRQn
#define STEP_Z_Pin GPIO_PIN_6
#define STEP_Z_GPIO_Port GPIOA
#define STEP_X_Pin GPIO_PIN_7
#define STEP_X_GPIO_Port GPIOA
#define STEP_Y_Pin GPIO_PIN_0
#define STEP_Y_GPIO_Port GPIOB
#define STEP_EXTR_Pin GPIO_PIN_1
#define STEP_EXTR_GPIO_Port GPIOB
#define EMERGENCY_STOP_IN_Pin GPIO_PIN_12
#define EMERGENCY_STOP_IN_GPIO_Port GPIOB
#define EMERGENCY_STOP_IN_EXTI_IRQn EXTI15_10_IRQn
#define EMERGENCY_STOP_OUT_Pin GPIO_PIN_13
#define EMERGENCY_STOP_OUT_GPIO_Port GPIOB
#define DIR_EXTR_Pin GPIO_PIN_15
#define DIR_EXTR_GPIO_Port GPIOA
#define DIR_Y_Pin GPIO_PIN_3
#define DIR_Y_GPIO_Port GPIOB
#define DIR_X_Pin GPIO_PIN_4
#define DIR_X_GPIO_Port GPIOB
#define DIR_Z_Pin GPIO_PIN_5
#define DIR_Z_GPIO_Port GPIOB
#define HEATER_BED_Pin GPIO_PIN_6
#define HEATER_BED_GPIO_Port GPIOB
#define HEATER_PRINT_HEAD_Pin GPIO_PIN_7
#define HEATER_PRINT_HEAD_GPIO_Port GPIOB
#define FAN_PRINTOUT_Pin GPIO_PIN_8
#define FAN_PRINTOUT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
typedef enum
{
    STATE_INIT = 0,
    STATE_RUN,
    STATE_STOP,
	STATE_HOMING
} SystemState_t;

typedef struct
{
    int dir;
    uint32_t period;
    int stepNumber;
} Velocity;


extern volatile SystemState_t currentState;


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
