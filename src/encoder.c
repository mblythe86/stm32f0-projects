/*
 * encoder.c
 *
 *  Created on: Oct 27, 2012
 *      Author: matt
 */

#include "encoder.h"

// Pin Definitions:

// PA0 - Timer 2 Channel 1 (AF2)
// PA1 - Timer 2 Channel 2 (AF2)
// PA6 - Timer 3 Channel 1 (AF1)
// PA7 - Timer 3 Channel 2 (AF1)

void encoder_init() {
  // Enable the GPIO Clocks
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  // configure pins
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_2);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_1);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_1);

  // Enable Timer clocks
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  // configure timers
  TIM_EncoderInterfaceConfig(TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
  TIM_EncoderInterfaceConfig(TIM3, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);

  TIM_Cmd(TIM2, ENABLE);
  TIM_Cmd(TIM3, ENABLE);
  // initialize counter values?
}

uint32_t get_position(TIM_TypeDef* TIMx) {
  return TIM_GetCounter(TIMx);
}
