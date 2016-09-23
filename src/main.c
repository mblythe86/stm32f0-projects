#include "stm32f0xx_conf.h"
#include "lcd.h"
#include "encoder.h"
#include "servo.h"
#include "pid.h"
#include "ir_remote.h"

static __IO uint32_t TimingDelay;

/**
 * @brief  Inserts a delay time.
 * @param  nTime: specifies the delay time length, in 1 ms.
 * @retval None
 */
void delay_ms(__IO uint32_t nTime) {
  TimingDelay = nTime*10;

  while (TimingDelay != 0)
    ;
}

/**
 * @brief  Decrements the TimingDelay variable.
 * @param  None
 * @retval None
 */
void TimingDelay_Decrement(void) {
  if (TimingDelay != 0x00) {
    TimingDelay--;
  }
}

//void PwmStuff(void) {
////  pwm_add_width(65,1);
////  pwm_sub_width(65,2);
//  pwm_add_width(30,1);
//  pwm_sub_width(30,2);
//}

int PidCount = 0;
int PwmCount = 0;
PidType pid1;
PidType pid2;

void SysTick_Handler(void) {
  //this will be executed every 0.1 ms
  //This probably means that we need to do less computation in
  //  the interrupt handler itself!

  //do_ir();

  //only do this every 100 ms (0.1s)
  if(PidCount++ == 1000){
    PidCount = 0;
//    PwmCount += 10;
//    if(PwmCount > 2000){
//      PwmCount = 1000;
//    }
//    servo_set_pos(PwmCount,1);
//    servo_set_pos(PwmCount,2);

    pid1.myInput = (int16_t)get_position(TIM2); //FIXME
    pid2.myInput = (int16_t)get_position(TIM3); //FIXME
    PID_Compute(&pid1);
    PID_Compute(&pid2);
    servo_set_pos(pid1.myOutput + 1500, 1);
    servo_set_pos(pid2.myOutput + 1500, 2);

//    int16_t kp = get_position(TIM2);
//    kp *= 0x10;
//    if(kp!=pid1.kp && kp > 0){
//      PID_SetTunings(&pid1, kp, 0, 0);
//    }

//    pid2.myInput = get_position(TIM3);
//    PID_Compute(&pid2);
//    pwm_set_width(pid2.myOutput, 2);
  }
  TimingDelay_Decrement();
//  PwmStuff();
}

typedef enum {
  SPIDER_AUTO_NEXT = 0,
  SPIDER_AUTO_WAIT = 1,
  SPIDER_MANUAL = 2,
  SPIDER_RECORD = 3,
  SPIDER_GO = 4
} Spider_state_t;

int main(void) {
  /* SysTick end of count event each 0.1ms */
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 10000);

  int kp = 20;
  int ki = 0;//kp/20;
  int kd = 0;//kp/4;
  PID_init(&pid1, kp, ki, kd, PID_Direction_Direct);
  PID_init(&pid2, kp, ki, kd, PID_Direction_Reverse);
  PID_SetOutputLimits(&pid1, -500, 500);
  PID_SetOutputLimits(&pid2, -500, 500);
//  PID_init(&pid1, 0, 0, 0, PID_Direction_Direct);
  lcd_init();
  encoder_init();
  servo_init();
  ir_init();
//  pwm_set_width(0, 1);
//  pwm_set_width(0, 2);
  //turn the PID on
  PID_SetMode(&pid1,PID_Mode_Automatic);
  PID_SetMode(&pid2,PID_Mode_Automatic);

//  GPIO_InitTypeDef GPIO_InitStructure;
//  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//  GPIO_Init(GPIOC, &GPIO_InitStructure);

  //to set a pin:
//  GPIOC->BSRR = GPIO_Pin_0;

  //to clear a pin:
//  GPIOA->BRR = GPIO_Pin_0;
  //or
//  GPIOA->BSRR = ((uint32_t) GPIO_Pin_0 << 16);

  //to toggle a pin:
//  GPIOA->ODR ^= GPIO_Pin_0;

//  RCC->AHBENR |= RCC_AHBENR_GPIOCEN; // enable the clock to GPIOC
//  //(RM0091 lists this as IOPCEN, not GPIOCEN)
//
//  GPIOC->MODER = (1 << 16);


  // 100 "setpoint units" ~= 8 3/8 inches
  int counter = 0;
  int savepoint = 0;
  int last_code = IR_ZERO;
  int tmp;
  int savepoints1[10], savepoints2[10];
  for(int i=0; i<10; i++){
    savepoints1[i] = 0;
    savepoints2[i] = 0;
  }
  pid1.mySetpoint = 0;
  pid2.mySetpoint = 0;
  Spider_state_t state = SPIDER_MANUAL;
  while (1){
    tmp = get_ir_code();
    if(tmp != IR_NOT_NEW){
      last_code = tmp;
      if(tmp == IR_PLAY){
        state = SPIDER_AUTO_NEXT;
				savepoint = 0;
      }
      else if(tmp == IR_STOP){
        state = SPIDER_MANUAL;
      }
      else if(state == SPIDER_MANUAL){
        if(tmp == IR_VOL_UP){
          pid2.mySetpoint += 10;
        }
        else if(tmp == IR_VOL_DN){
          pid2.mySetpoint -= 10;
        }
        else if(tmp == IR_CHN_DN){
          pid1.mySetpoint -= 10;
        }
        else if(tmp == IR_CHN_UP){
          pid1.mySetpoint += 10;
        }
        else if(tmp == IR_REC){
          state = SPIDER_RECORD;
        }
        else if(tmp == IR_GO){
          state = SPIDER_GO;
        }

        if(pid1.mySetpoint > pid1.myInput + 20){
          pid1.mySetpoint = pid1.myInput + 20;
        }
        else if(pid1.mySetpoint < pid1.myInput - 20){
          pid1.mySetpoint = pid1.myInput - 20;
        }
        if(pid2.mySetpoint > pid2.myInput + 20){
          pid2.mySetpoint = pid2.myInput + 20;
        }
        else if(pid2.mySetpoint < pid2.myInput - 20){
          pid2.mySetpoint = pid2.myInput - 20;
        }
      }
      else if(state == SPIDER_RECORD){
        if(tmp == IR_REC){
          //do nothing, easy to hit twice
        }
        else if (tmp >= IR_ZERO && tmp <= IR_NINE) {
          savepoints1[tmp] = pid1.mySetpoint;
          savepoints2[tmp] = pid2.mySetpoint;
          state = SPIDER_MANUAL;
        }
        else{
          state = SPIDER_MANUAL;
        }
      }
      else if(state == SPIDER_GO){
        if(tmp == IR_GO){
          //do nothing, easy to hit twice
        }
        else if (tmp >= IR_ZERO && tmp <= IR_NINE) {
          pid1.mySetpoint = savepoints1[tmp];
          pid2.mySetpoint = savepoints2[tmp];
          state = SPIDER_MANUAL;
        }
        else{
          state = SPIDER_MANUAL;
        }
      }
    }
    
    if(state == SPIDER_AUTO_NEXT){
      if (  savepoints1[savepoint] != 0 
         && savepoints2[savepoint] != 0){
        pid1.mySetpoint = savepoints1[savepoint];
        pid2.mySetpoint = savepoints2[savepoint];
        counter = 0;
        state = SPIDER_AUTO_WAIT;
      }
      else{
        savepoint++;
        if(savepoint > 9){
          savepoint = 0;
        }
      }
    }
    if(state == SPIDER_AUTO_WAIT){
      int diff1 = savepoints1[savepoint] - pid1.myInput;
      int diff2 = savepoints2[savepoint] - pid2.myInput;
      if (  diff1 < 10 && diff1 > -10
         && diff2 < 10 && diff2 > -10){
         counter++;
      }
      if(counter > 60){ //about 6 sec?
        counter = 0;
        savepoint++;
        if(savepoint > 9){
          savepoint = 0;
        }
        state = SPIDER_AUTO_NEXT;
      }
    }

    lcd_line_one();
    if(1){
    //    lcd_write_string("TIM2: ");
//    lcd_write_int16(get_position(TIM2));
//    lcd_write_string("  ");
//    lcd_line_two();
    lcd_write_string("TIM3: ");
    int16_t pos = get_position(TIM3);
    lcd_write_int16(pos);
    lcd_write_string(" TIM2: ");
    pos = get_position(TIM2);
    lcd_write_int16(pos);
    lcd_write_string(" ");
    lcd_line_two();
    lcd_write_string("state: ");
    lcd_write_int16(state);
    lcd_write_string(" pt: ");
    lcd_write_int16(savepoint);
    lcd_write_string("   ");
    lcd_line_three();
    lcd_write_string("counter: ");
    lcd_write_int16(counter);
    lcd_write_string("   ");
    lcd_line_four();
    lcd_write_string("IR code: ");
    lcd_write_string(get_code_string(last_code));
    lcd_write_string(" ");
    lcd_write_int16(get_ir_state());
    lcd_write_string(" ");
    lcd_write_int16(get_ir_bits_read());
    //lcd_write_string(" ");
    //lcd_write_int16_hex(get_ir_code());
    lcd_write_string("       ");
    }
    else{
    for(int i=0; i<20 && i<idx; i++){
      lcd_write_int16(record[i]>>8);
    }
    lcd_line_two();
    for(int i=0; i<20 && i<idx; i++){
      if(record[i] & 0xf0){
        lcd_write_string("Z");
      }
      else if((record[i] & 0xf) <= 9){
        lcd_write_int16(record[i]&0xf);
      }
      else{
        lcd_write_data((record[i]&0xf) - 0xa + 'A');
      }
    }
    lcd_line_three();
    for(int i=20; i<40 && i<idx; i++){
      lcd_write_int16(record[i]>>8);
    }
    lcd_write_string("       ");
    lcd_line_four();
    for(int i=20; i<40 && i<idx; i++){
      if(record[i] & 0xf0){
        lcd_write_string("Z");
      }
      else if((record[i] & 0xf) <= 9){
        lcd_write_int16(record[i]&0xf);
      }
      else{
        lcd_write_data((record[i]&0xf) - 0xa + 'A');
      }
    }
    lcd_write_string("       ");
    }
    delay_ms(100);

  }

}
