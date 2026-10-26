#include "fake_axis.h"

Axis axis_map[AXIS_COUNT];

void process_axis(axis_t axis) { (void)axis; }
void axis_init(void *tim_handler) { (void)tim_handler; }
