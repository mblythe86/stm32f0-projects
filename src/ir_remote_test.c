/*
 * ir_remote.h
 *
 *  Created on: Sept 30, 2015
 *      Author: matt
 */
#include <stdint.h>
#include <stdio.h>

/*
all signals are active low.  i.e. 1=pulled to ground
each digit represents approximately 880 microseconds (us)
Also, all are preceeded and followed by (mostly omitted) zeroes
Here's the pattern: each one starts with "101" as a header,
Then it's followed by 2-bit pairs of either
"01" (call this 1) or "10" (call this 0). This is Manchester coding.
Each signal below actually encodes 12 bits (24 transitions)
OK     : 101010101011010011010011001 - 111100100101 - 0xf25
Up     : 101100101011010100110011010 - 011100010100 - 0x714
Down   : 101010101011010100110011001 - 111100010101 - 0xf15
Left   : 101100101011010100110010110 - 011100010110 - 0x716
Right  : 101010101011010100110010101 - 111100010111 - 0xf17
Vol up : 101010101011010100110101010 - 111100010000 - 0xf10
Vol dn : 101100101011010100110101001 - 011100010001 - 0x711
Chn up : 101010101011010011010101010 - 111100100000 - 0xf20
Chn dn : 101100101011010011010101001 - 011100100001 - 0x721

*/

#ifndef IR_REMOTE_H_
#define IR_REMOTE_H_

typedef enum{
  NO_RESULT,
  RESULT,
  IGN_ERROR
} Ir_return_t;

typedef enum{
  IR_ZERO   = 0x00,
  IR_ONE    = 0x01,
  IR_TWO    = 0x02,
  IR_THREE  = 0x03,
  IR_FOUR   = 0x04,
  IR_FIVE   = 0x05,
  IR_SIX    = 0x06,
  IR_SEVEN  = 0x07,
  IR_EIGHT  = 0x08,
  IR_NINE   = 0x09,
  IR_PWR    = 0x3d,
  IR_GO     = 0x3b,
  IR_OK     = 0x25,
  IR_UP     = 0x14,
  IR_DOWN   = 0x15,
  IR_LEFT   = 0x16,
  IR_RIGHT  = 0x17,
  IR_VOL_UP = 0x10,
  IR_VOL_DN = 0x11,
  IR_CHN_UP = 0x20,
  IR_CHN_DN = 0x21,
  IR_BACK   = 0x1f,
  IR_MENU   = 0x0d,
  IR_TV     = 0x1c,
  IR_MUTE   = 0x0f,
  IR_STOP   = 0x36,
  IR_SKIP_PREV = 0x24,
  IR_REWIND = 0x32,
  IR_PAUSE  = 0x30,
  IR_PLAY   = 0x35,
  IR_FF     = 0x34,
  IR_SKIP_NEXT = 0x1e,
  IR_REC    = 0x37,
  IR_PREV   = 0x12,
  IR_TEXT   = 0x0a,
  IR_CC     = 0x0e,
  IR_NOT_NEW = 0xff
} Ir_button_t;

typedef enum{
  IDLE,
  HEADER1,
  HEADER2,
//  HEADER3,  This is logically equivalent to MANCHESTER2
  MANCHESTER1,
  MANCHESTER2,
  IR_ERROR
} Ir_state_t;

extern volatile int32_t record[40];
extern volatile int idx;

void        ir_init();
void        EXTI4_15_IRQHandler();
void        do_ir();
Ir_state_t  get_ir_state();
int         get_ir_bits_read();
int         get_ir_code();
const char* get_code_string(Ir_button_t);

#endif /* IR_REMOTE_H */
/*
 * ir_remote.c
 *
 *  Created on: Sept 30, 2015
 *      Author: matt
 */

// PIN DEFINITIONS:

// PA5 -- IR decoder output (active low)



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

void do_ir(int pin, int delay){
  //We expect this to be called by the pin change interrupt

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

void do_ir_print(int pin, int delay){
  do_ir(pin, delay*100);
  printf("(%d,%d) -> %d %d\n", pin, delay, Ir_state, Bits_read);
}

int main(){
  do_ir_print(0, 0);
  do_ir_print(1, 8);
  do_ir_print(0,8);
  do_ir_print(1,17);
  do_ir_print(0,17);
  do_ir_print(1,8);
  do_ir_print(0,8);
  do_ir_print(1,8);
  do_ir_print(0,8);
  do_ir_print(1,17);
  do_ir_print(0,8);
  do_ir_print(1,8);
  do_ir_print(0,17);
  do_ir_print(1,17);
  do_ir_print(0,8);
  do_ir_print(1,8);
  do_ir_print(0,17);
  do_ir_print(1,17);
  do_ir_print(0,17);
  do_ir_print(1,8);

  return 0;
}
