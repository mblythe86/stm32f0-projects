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
}

typedef enum{
  IDLE,
  HEADER1,
  HEADER2,
//  HEADER3,  This is logically equivalent to MANCHESTER2
  MANCHESTER1,
  MANCHESTER2,
  NEW_ERROR,
  IR_ERROR
} Ir_state_t;

enum{
  SHORT_MIN = 6,
  SHORT_MAX = 12,
  LONG_MIN = 15,
  LONG_MAX = 20
};

Ir_state_t Ir_state = IDLE;
int Bits_read = 0;
int Ir_code = 0;
int Ir_code_next = 0;
int Last_pin = 1; //Since this signal is active-low, 1 is "reset"
int Call_count = 0;
int Ir_valid_code;

void ir_short(int pin);
void ir_long(int pin);

int get_ir_code(){
  return Ir_code;
}

Ir_return_t do_ir(){
  Ir_valid_code = 0;
  //We expect this to be called every 0.1ms

  //read pin
  int pin = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5); //FIXME!!!

  switch(Ir_state){
    case IDLE:
      if(pin == 0){
        Ir_state = HEADER1;
        Call_count = 0;
      }
      break;
    case HEADER1:
      if(Call_count > SHORT_MAX){
        Ir_state = NEW_ERROR;
      }
      else if(pin == 1){
        if(Call_count < SHORT_MIN){
          Ir_state = NEW_ERROR;
        }
        else{
          Ir_state = HEADER2;
          Call_count = 0;
        }
      }
      break;
    case HEADER2:
      if(Call_count > SHORT_MAX){
        Ir_state = NEW_ERROR;
      }
      else if(pin == 0){
        if(Call_count < SHORT_MIN){
          Ir_state = NEW_ERROR;
        }
        else{
          Ir_state = MANCHESTER2;
          Call_count = 0;
          Bits_read = 0;
          Ir_code_next = 0;
        }
      }
      break;
    case MANCHESTER1:
    case MANCHESTER2:
      if(pin != Last_pin){
        switch(Call_count){
          case 6 ... 12: //centered on 9 plus/minus 3 CAREFUL! this is a gcc extension!
            ir_short(pin);
            break;
          case 15 ... 20: //centered on 18 plus/minus 3 CAREFUL! this is a gcc extension!
            ir_long(pin);
            break;
          default:
            Ir_state = NEW_ERROR;
        }
        Call_count = 0;
      }
      else if(Call_count > 50){
        if(pin == 1 && Bits_read == 12){
          Ir_valid_code = 1;
          Ir_code = Ir_code_next;
          Ir_state = IDLE;
        }
        else{
          Ir_state = NEW_ERROR;
        }
      }
      break;
    case NEW_ERROR:
      Ir_state = IR_ERROR;
      //fallthrough
    default: //case IR_ERROR:
      if(pin == 0){
        Call_count = 0;
      }
      if(pin == 1 && Call_count > 50){
        Ir_state = IDLE;
      }
  }
  Last_pin = pin;
  Call_count++;
  if(Ir_state == NEW_ERROR){
    return IGN_ERROR;
  }
  else if(Ir_valid_code){
    return RESULT;
  }
  else{
    return NO_RESULT;
  }
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
    case IR_OK: return "OK";
    case IR_UP: return "UP";
    case IR_DOWN: return "DOWN";
    case IR_LEFT: return "LEFT";
    case IR_RIGHT: return "RIGHT";
    case IR_VOL_UP: return "VOL_UP";
    case IR_VOL_DN: return "VOL_DN";
    case IR_CHN_UP: return "CHN_UP";
    case IR_CHN_DN: return "CHN_DN";
    default: return "UNKWN";
  }
}
