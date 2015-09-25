/*
 * pwm.h
 *
 *  Created on: Oct 27, 2012
 *      Author: matt
 */

#ifndef SERVO_H_
#define SERVO_H_

#include "stm32f0xx_conf.h"

void servo_init();

void servo_set_pos(uint32_t pos, uint32_t servo);

#endif /* SERVO_H_ */
