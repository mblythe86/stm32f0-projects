#include "stm32f0xx_conf.h"
#include "lcd.h"
#include "encoder.h"
#include "pwm.h"

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

void PwmStuff(void) {
//  pwm_add_width(65,1);
//  pwm_sub_width(65,2);
  pwm_add_width(30,1);
  pwm_sub_width(30,2);
}

void SysTick_Handler(void) {
  TimingDelay_Decrement();
  PwmStuff();
}

int main(void) {
  /* SysTick end of count event each 1ms */
  RCC_ClocksTypeDef RCC_Clocks;
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);

  lcd_init();
  encoder_init();
  pwm_init();

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



  while (1){
    lcd_line_one();
    lcd_write_string("TIM2: ");
    lcd_write_int16(get_position(TIM2));
    lcd_write_string("  ");
    lcd_line_two();
    lcd_write_string("TIM3: ");
    lcd_write_int16(get_position(TIM3));
    lcd_write_string("  ");
    delay_ms(100);
  }

}
