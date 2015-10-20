/*
 * lcd.c
 *
 *  Created on: Oct 13, 2012
 *      Author: matt
 */

// PIN DEFINITIONS:

// PC13  -- LCD Register Select (RS). RS=0: Command, RS=1: Data
// PC14  -- LCD Read/Write (R/W). R/W=0: Write, R/W=1: Read
// PC15  -- LCD Clock (Enable). Falling edge triggered
// PC0-3 -- LCD Data Bits 4-7

#include "lcd.h"
#include "stm32f0xx_conf.h"
void delay_ms(__IO uint32_t nTime);

int8_t lcd_column;

void delay_loop(int a) {
  volatile int i, j;
  for (i = 0; i < a; i++) {
    j++;
  }
}

// lcd_set_type_data()
void lcd_set_type_data() {
//  PORTD |= (1<<PD7);
  GPIOC->BSRR = GPIO_Pin_13;
  delay_loop(3);
}

// lcd_set_type_command()
void lcd_set_type_command() {
//  PORTD &= ~(1<<PD7);
  GPIOC->BRR = GPIO_Pin_13;
  delay_loop(3);
}

// lcd_write_nibble(...)
void lcd_write_nibble(char c) {

  // NOTE: only 2 or 3 work in the delays here.

  // set data
//  PORTD &= ~(0x0f << 2);
//  PORTD |= (c&0x0f) << 2;
  uint32_t set_bits = (c & 0xf);
  uint32_t clr_bits = (~c & 0xf) <<16;
  GPIOC->BSRR = (set_bits | clr_bits);

  // E high
//  PORTD |= (1<<PD6);
//  delay_us(1);
  GPIOC->BSRR = GPIO_Pin_15;
  delay_loop(50);

  // E low
//  PORTD &= ~(1<<PD6);
//  delay_us(1);
  GPIOC->BRR = GPIO_Pin_15;
  delay_loop(50);
}

void lcd_write_byte(char c) {
  lcd_write_nibble( (c >> 4) & 0x0f );
  lcd_write_nibble( c & 0x0f );
//  delay_us(80);
  delay_loop(50*80);
}

void lcd_clear_and_home() {
  lcd_set_type_command();
  lcd_write_byte(0x01);
  delay_ms(50);
  lcd_write_byte(0x02);
  delay_ms(50);
  lcd_column = 0;
}

void lcd_home() {
  lcd_set_type_command();
  lcd_write_byte(0x02);
  delay_ms(50);
  lcd_column = 0;
}

void lcd_write_data(char c) {
  if( lcd_column <20){
    lcd_set_type_data();
    lcd_write_byte(c);
    lcd_column++;
  }
}

// lcd_write_int16
void lcd_write_int16(int16_t in) {
  uint8_t started = 0;

  uint16_t pow = 10000;

  if(in < 0) {
    lcd_write_data('-');
    in = -in;
  }

  while(pow >= 1) {
    if(in / pow > 0 || started || pow==1) {
      lcd_write_data((uint8_t) (in/pow) + '0');
      started = 1;
      in = in % pow;
    }

    pow = pow / 10;
  }

}

// lcd_write_int16
void lcd_write_int16_hex(uint16_t in) {
  uint8_t started = 0;
  lcd_write_data('0');
  lcd_write_data('x');
  for(int16_t place = 12; place >= 0; place -= 4){
    uint16_t digit = (in >> (place)) & 0xf;
    if(digit >= 0xa){
      lcd_write_data((uint8_t) (digit-0xa) + 'a');
    }
    else{
      lcd_write_data((uint8_t) (digit) + '0');
    }
  }
}


// lcd_write_int16_centi
// assumes that its measured in centi-whatevers
void lcd_write_int16_centi(int16_t in) {
  uint8_t started = 0;

  uint16_t pow = 10000;

  if(in < 0) {
    lcd_write_data('-');
    in = -in;
  }

  while(pow >= 1) {
    if(in / pow > 0 || started || pow==1) {
      lcd_write_data((uint8_t) (in/pow) + '0');
      started = 1;
      in = in % pow;
    }

    if(pow == 100) {
      if(!started) {
        lcd_write_data('0');
      }
      lcd_write_data('.');
      started = 1;
    }

    pow = pow / 10;
  }

}

void lcd_write_string(const char *x) {
  // assumes x is in program memory
//  while(pgm_read_byte(x) != 0x00)
//    lcd_write_data(pgm_read_byte(x++));
  while((*x) != 0x00){
    lcd_write_data((*x));
    x++;
  }
}

void lcd_goto_position(uint8_t row, uint8_t col) {
  lcd_set_type_command();

  // 20x4 LCD: offsets 0, 0x40, 20, 0x40+20
  uint8_t row_offset = 0;
  switch(row) {
    case 0: row_offset = 0; break;
    case 1: row_offset = 0x40; break;
    case 2: row_offset = 20; break;
    case 3: row_offset = 0x40+20; break;
  }


  lcd_write_byte(0x80 | (row_offset + col));
  lcd_column = col;
}

void lcd_line_one()   { lcd_goto_position(0, 0); }
void lcd_line_two()   { lcd_goto_position(1, 0); }
void lcd_line_three() { lcd_goto_position(2, 0); }
void lcd_line_four()  { lcd_goto_position(3, 0); }

// lcd_init()
void lcd_init() {
  delay_ms(100);
  // set pin driver directions
  // (output on PD7,PD6, and PD3-6)
//  DDRD |= 0xfc;

  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable the GPIO Clocks */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
  //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  /* Configure the GPIO_LED pins */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  // wait 100msec
  delay_ms(100);
  lcd_set_type_command();

  // do reset
  lcd_write_nibble(0x03);
  delay_ms(6);
  lcd_write_nibble(0x03);
  delay_loop(250*50);
  lcd_write_nibble(0x03);
  delay_loop(250*50);

  // write 0010 (data length 4 bits)
  lcd_write_nibble(0x02);
  delay_loop(50*80);
  // set to 2 lines, font 5x8
  lcd_write_byte(0x28);
  // disable LCD
  //lcd_write_byte(0x08);
  // enable LCD
  lcd_write_byte(0x0c);
  // clear display
  lcd_write_byte(0x01);
  delay_ms(5);
  // enable LCD
  lcd_write_byte(0x0c);
  // set entry mode
  lcd_write_byte(0x06);

  // set cursor/display shift
  lcd_write_byte(0x14);

  // clear and home
  lcd_clear_and_home();
}

//int lcd_putchar(char c, FILE *stream) {
//  lcd_write_data(c);
//  return 0;
//}
