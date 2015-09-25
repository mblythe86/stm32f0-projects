/*
 * servo.c
 *
 *  Created on: Oct 27, 2012
 *      Author: matt
 */

#include "servo.h"

// Pin Definitions:

// PA2 - Timer 15 Channel 1 (AF0)
// PA3 - Timer 15 Channel 2 (AF0)

void servo_init() {
  // Enable the GPIO Clocks
  RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOA, ENABLE);

  // configure pins
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_0);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_0);

  // Enable Timer clocks
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);

  /* Time Base configuration */
  /*
   * ok, so we want a 1-2ms pulse every 20ms
   * the internal clock is 48MHz, so if we divide by a /48 prescaler,
   * we get a 1MHZ counter clock so,
   *  20ms repeat = 20,000 count
   *   1ms pulse = 1000 count
   * 1.5ms pulse = 1500 count
   *   2ms pulse = 2000 count
   */
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Prescaler = 47; //48 = 47+1
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 20000;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

  TIM_TimeBaseInit(TIM15, &TIM_TimeBaseStructure);

  /* Channel 1, 2, 3 and 4 Configuration in PWM mode */
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

  TIM_OCInitStructure.TIM_Pulse = 1500;
  TIM_OC1Init(TIM15, &TIM_OCInitStructure);

  TIM_OCInitStructure.TIM_Pulse = 1500;
  TIM_OC2Init(TIM15, &TIM_OCInitStructure);

  /* TIM1 counter enable */
  TIM_Cmd(TIM15, ENABLE);

  /* TIM1 Main Output Enable */
  TIM_CtrlPWMOutputs(TIM15, ENABLE);
}

void servo_set_pos(uint32_t pos, uint32_t servo){
  if(servo == 1){
//    TIM_SetCompare2(TIM3, pos);
    TIM15->CCR1 = pos;
  }
  else if(servo == 2){
    TIM15->CCR2 = pos;
  }
  else{
    //error?
  }
}

