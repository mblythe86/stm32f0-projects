#include "stm32f0xx_conf.h"
#include "lcd.h"
#include "encoder.h"
#include "pwm.h"
#include "pid.h"

static __IO uint32_t TimingDelay;

/**
 * @brief  Inserts a delay time.
 * @param  nTime: specifies the delay time length, in 1 ms.
 * @retval None
 */
void delay_ms(__IO uint32_t nTime) {
  TimingDelay = nTime;

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
PidType pid1;
PidType pid2;

void SysTick_Handler(void) {
  //only do this every 100 ms (0.1s)
  if(PidCount++ == 10){
    PidCount = 0;

    pid1.myInput = get_position(TIM3); //FIXME
    PID_Compute(&pid1);
    pwm_set_width(pid1.myOutput, 1);


    pid2.myInput = get_position(TIM3);
    PID_Compute(&pid2);
    pwm_set_width(pid2.myOutput, 2);
  }
  TimingDelay_Decrement();
//  PwmStuff();
}

int main(void) {
  /* SysTick end of count event each 1ms */
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

  PID_init(&pid1, 0x100, 0x20, 0x100, PID_Direction_Direct);
  PID_init(&pid2, 10, 2, 1, PID_Direction_Direct);
  lcd_init();
  encoder_init();
  pwm_init();
  pwm_set_width(0, 1);
  pwm_set_width(0, 2);
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


  int counter = 0;
  while (1){
    lcd_line_one();
//    lcd_write_string("TIM2: ");
//    lcd_write_int16(get_position(TIM2));
//    lcd_write_string("  ");
//    lcd_line_two();
    lcd_write_string("TIM3: ");
    lcd_write_int16(get_position(TIM3));
    lcd_write_string("  ");
    lcd_line_two();
    lcd_write_string("pid set: ");
    lcd_write_int16(pid1.mySetpoint);
    lcd_write_string("   ");
    lcd_line_three();
    lcd_write_string("pid in:  ");
    lcd_write_int16(pid1.myInput);
    lcd_write_string("   ");
    lcd_line_four();
    lcd_write_string("pid out: ");
    lcd_write_int16(pid1.myOutput);
    lcd_write_string("   ");
    if(counter++ == 40){
      pid1.mySetpoint = 100;
      pid2.mySetpoint = 100;
    }else if(counter == 80){
      pid1.mySetpoint = 0;
      pid2.mySetpoint = 0;
      counter = 0;
    }
    delay_ms(100);
  }

}
