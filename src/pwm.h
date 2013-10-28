/*
 * pwm.h
 *
 *  Created on: Oct 27, 2012
 *      Author: matt
 */

#ifndef PWM_H_
#define PWM_H_

#include "stm32f0xx_conf.h"

void pwn_init();

void pwm_set_width(uint32_t width, uint32_t channel);
void pwm_add_width(uint32_t width, uint32_t channel);
void pwm_sub_width(uint32_t width, uint32_t channel);

#endif /* PWM_H_ */
