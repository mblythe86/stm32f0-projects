/*
 * ir_remote.h
 *
 *  Created on: Sept 30, 2015
 *      Author: matt
 */

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

void        ir_init();
void        EXTI4_15_IRQHandler();
void        do_ir();
Ir_state_t  get_ir_state();
int         get_ir_bits_read();
int         get_ir_code();
const char* get_code_string(Ir_button_t);

#endif /* IR_REMOTE_H */
