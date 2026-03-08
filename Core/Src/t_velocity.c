#include "t_velocity.h";

uint8_t vel_get_dir(t_velocity vel){
	return vel & 0x00000003;
}
uint16_t vel_get_period(t_velocity vel){
	return vel & 0xFFFF0000;
}
uint16_t vel_get_step_number(t_velocity vel){
	return vel & 0x0000FFFC;
}
