/*
 * pwm.c
 *
 *  Created on: Oct 27, 2012
 *      Author: matt
 */

#include "pwm.h"

// Pin Definitions:

// PA2 - Timer 15 Channel 1 (AF0)
// PA3 - Timer 15 Channel 2 (AF0)

void pwm_init() {
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
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 0xffff;
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
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Set;

  TIM_OCInitStructure.TIM_Pulse = 0;
  TIM_OC1Init(TIM15, &TIM_OCInitStructure);

  TIM_OCInitStructure.TIM_Pulse = 0xffff;
  TIM_OC2Init(TIM15, &TIM_OCInitStructure);

  /* TIM1 counter enable */
  TIM_Cmd(TIM15, ENABLE);

  /* TIM1 Main Output Enable */
  TIM_CtrlPWMOutputs(TIM15, ENABLE);
}

void pwm_set_width(uint32_t width, uint32_t channel){
  if(channel == 1){
//    TIM_SetCompare1(TIM15, width);
    TIM15->CCR1 = width;
  }
  else if(channel == 2){
//    TIM_SetCompare2(TIM15, width);
    TIM15->CCR2 = width;
  }
  else{
    //error?
  }
}

void pwm_add_width(uint32_t width, uint32_t channel){
  uint32_t val;
  if(channel == 1){
    val = TIM15->CCR1 + width;
    if(val > 0xffff){
      val = 0;
    }
    TIM15->CCR1 = val;
  }
  else if(channel == 2){
    val = TIM15->CCR2 + width;
    if(val > 0xffff){
      val = 0;
    }
    TIM15->CCR2 = val;
  }
  else{
    //error?
  }
}

void pwm_sub_width(uint32_t width, uint32_t channel){
  uint32_t val;
  if(channel == 1){
    if(TIM15->CCR1 < width){
      TIM15->CCR1 = 0xffff;
    }
    else {
      TIM15->CCR1 = TIM15->CCR1 - width;
    }
  }
  else if(channel == 2){
    if(TIM15->CCR2 < width){
      TIM15->CCR2 = 0xffff;
    }
    else {
      TIM15->CCR2 = TIM15->CCR2 - width;
    }
  }
  else{
    //error?
  }
}
