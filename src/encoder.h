/*
 * encoder.h
 *
 *  Created on: Oct 27, 2012
 *      Author: matt
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#include "stm32f0xx_conf.h"

void encoder_init();
uint32_t get_position(TIM_TypeDef* TIMx);

#endif /* ENCODER_H_ */
