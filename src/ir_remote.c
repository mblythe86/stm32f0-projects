/*
 * ir_remote.c
 *
 *  Created on: Sept 30, 2015
 *      Author: matt
 */

// PIN DEFINITIONS:

// PA5 -- IR decoder output (active low)

#include "ir_remote.h"

void ir_init() {
  // Enable the GPIO Clocks
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  // configure pins
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  //set up pin interrupt on PA5
  SYSCFG->EXTICR[2] &= ~(SYSCFG_EXTICR2_EXTI5_PA); //clear the register
  SYSCFG->EXTICR[2] |= SYSCFG_EXTICR2_EXTI5; //set it to listen to PA5
  EXTI->RTSR |= EXTI_RTSR_TR5; //trigger on rising edge
  EXTI->FTSR |= EXTI_FTSR_TR5; //trigger on falling edge
  EXTI->IMR |= EXTI_IMR_MR5; //unmask interrupt
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 1; //0 highest, 3 lowest
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  //we'll use timer TIM16 to see how much time has elapsed between interrupts
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM16, ENABLE);
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Prescaler = 47; //48 = 47+1...counting at 1MHz
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM16, &TIM_TimeBaseStructure);

  TIM_SelectOnePulseMode(TIM16, TIM_OPMode_Single);

  //want to do this frequently:
  TIM_SetCounter(TIM16, 0);
  TIM_Cmd(TIM16, ENABLE);

  //stop the timer in debug mode
  DBGMCU->APB2FZ &= DBGMCU_APB2_FZ_DBG_TIM16_STOP;
  /* Configure the GPIO_LED pins */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void EXTI4_15_IRQHandler() {
  do_ir();

  //clear the pending interrupt
  EXTI->PR |= EXTI_PR_PR5;
}


enum{
  SHORT_MIN = 600,
  SHORT_MAX = 1200,
  LONG_MIN = 1500,
  LONG_MAX = 2000
};

Ir_state_t Ir_state = IDLE;
volatile int Bits_read = 0;
volatile int Ir_code = 0;
volatile int Ir_code_next = 0;
volatile int Last_pin = 1; //Since this signal is active-low, 1 is "reset"
volatile int32_t record[40];
volatile int idx;

void ir_short(int pin);
void ir_long(int pin);

Ir_state_t get_ir_state(){
  return Ir_state;
}

int get_ir_bits_read(){
  return Bits_read;
}

int get_ir_code(){
  int tmp = Ir_code;
  Ir_code = IR_NOT_NEW;
  return tmp;
}

void do_ir(){
  //We expect this to be called by the pin change interrupt

  //read pin
  int pin = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5); //FIXME!!!
  int delay = TIM_GetCounter(TIM16);
  TIM_SetCounter(TIM16, 0);
  TIM_Cmd(TIM16, ENABLE);

  if(delay ==0){
    //the timer has wrapped, start our record over
    idx = 0;
  }
  record[idx] = 0;
  record[idx] = pin <<8;
  record[idx] |= ((delay/100)-5)&0xff;
  idx++;

  //if(pin == Last_pin){
  //  GPIOB->ODR |= GPIO_Pin_2;
  //}
  //else{
  //  GPIOB->ODR &= ~(GPIO_Pin_2);
  //}

  if(delay > LONG_MAX){
    if(pin == 1 || Ir_state != IDLE){
      Ir_state = IDLE;
    }
  }
  else if(delay < SHORT_MIN || (delay > SHORT_MAX && delay < LONG_MIN)){
    if(pin == 1){
      Ir_state = IR_ERROR;
    }
    else{
      Ir_state = IDLE;
    }
  }
  switch(Ir_state){
  case IDLE:
    if(pin == 0){
      Ir_state = HEADER1;
    }
    break;
  case HEADER1:
    if(delay > SHORT_MAX){
      Ir_state = IR_ERROR;
    }
    else if(pin == 1){
      Ir_state = HEADER2;
    }
    break;
  case HEADER2:
    if(delay > SHORT_MAX){
      Ir_state = IR_ERROR;
    }
    else if(pin == 0){
      Ir_state = MANCHESTER2;
      Bits_read = 0;
      Ir_code_next = 0;
    }
    break;
  case MANCHESTER1:
  case MANCHESTER2:
    if(pin != Last_pin){
      if(delay >= SHORT_MIN && delay <= SHORT_MAX){
        ir_short(pin);
      }
      else if(delay >= LONG_MIN && delay <= LONG_MAX){
        ir_long(pin);
      }
      else{
        Ir_state = IR_ERROR;
      }
    }
    //check if we're done
    if(pin == 1 && Bits_read == 12){
      if( (Ir_code_next & 0x7c0) == 0x700){
        Ir_code = Ir_code_next & 0x3f;
        Ir_state = IDLE;
      }
      else{
        Ir_state = IR_ERROR;
      }
    }
    break;
  default: //case IR_ERROR:
    if(pin == 1){
      Ir_state = IDLE;
    }
  }
  Last_pin = pin;
}

void ir_short(int pin){
  if(Ir_state == MANCHESTER2){
    Ir_state = MANCHESTER1;
  }
  else{ //I'd better be in MANCHESTER1, which means that this bit is the one that matters
    Ir_state = MANCHESTER2;
    Bits_read++;
    Ir_code_next <<= 1;
    if(!pin){
      Ir_code_next |= 1;
    }
  }
}

void ir_long(int pin){
  if(Ir_state == MANCHESTER1){
    Ir_state = IR_ERROR;
  }
  else{
    //Ir_state remains unchanged at MANCHESTER2
    Bits_read++;
    Ir_code_next <<= 1;
    if(!pin){
      Ir_code_next |= 1;
    }
  }
}

const char* get_code_string(Ir_button_t code){
  switch(code){
    case IR_ZERO: return "ZERO";
    case IR_ONE: return "ONE";
    case IR_TWO: return "TWO";
    case IR_THREE: return "THREE";
    case IR_FOUR: return "FOUR";
    case IR_FIVE: return "FIVE";
    case IR_SIX: return "SIX";
    case IR_SEVEN: return "SEVEN";
    case IR_EIGHT: return "EIGHT";
    case IR_NINE: return "NINE";
    case IR_PWR: return "PWR";
    case IR_GO: return "GO";
    case IR_OK: return "OK";
    case IR_UP: return "UP";
    case IR_DOWN: return "DOWN";
    case IR_LEFT: return "LEFT";
    case IR_RIGHT: return "RIGHT";
    case IR_VOL_UP: return "VOL_UP";
    case IR_VOL_DN: return "VOL_DN";
    case IR_CHN_UP: return "CHN_UP";
    case IR_CHN_DN: return "CHN_DN";
    case IR_BACK: return "BACK";
    case IR_MENU: return "MENU";
    case IR_TV: return "TV";
    case IR_MUTE: return "MUTE";
    case IR_STOP: return "STOP";
    case IR_SKIP_PREV: return "SKIP_PREV";
    case IR_REWIND: return "REWIND";
    case IR_PAUSE: return "PAUSE";
    case IR_PLAY: return "PLAY";
    case IR_FF: return "FF";
    case IR_SKIP_NEXT: return "SKIP_NEXT";
    case IR_REC: return "REC";
    case IR_PREV: return "PREV";
    case IR_TEXT: return "TEXT";
    case IR_CC: return "CC";
    default: return "???";
  }
}
