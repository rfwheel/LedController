
#include <tim.h>
#include "led.h"

#define LED_TIM_ARR     159
#define LED_TIM_OC_ZERO 32
#define LED_TIM_OC_ONE  77
#define LED_TIM_OC_LOW  0xffff
#define LED_TIM_OC_HIGH 0

#define LED_GPIO_ALT_FUNC  (0b10 << GPIO_MODER_MODE10_Pos)
#define LED_GPIO_PA10_AF2  (0b0010 << GPIO_AFRH_AFSEL10_Pos)

#define LED_TIM_OC_OUTPUT  (0b00 << TIM_CCMR2_CC3S_Pos)
#define LED_TIM_OC_FAST    (TIM_CCMR2_OC3FE) 
#define LED_TIM_OC_PRELOAD (TIM_CCMR2_OC3PE) 
#define LED_TIM_OC_PWM1    (TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2) 
#define LED_TIM_OC_NOCLEAR (0b0 << TIM_CCMR2_OC3CE_Pos) 

void led_init() {
  uint32_t val;

  // PA10 config
  val = GPIOA->MODER;
  val &= ~GPIO_MODER_MODE10_Msk;
  val |= LED_GPIO_ALT_FUNC;
  GPIOA->MODER = val;

  val = GPIOA->AFR[1];
  val &= ~GPIO_AFRH_AFSEL10_Msk;
  val |= LED_GPIO_PA10_AF2;
  GPIOA->AFR[1] = val;

  // TIM1 CH3 config
  val = TIM1->CCMR2;
  val &= ~(TIM_CCMR2_CC3S_Msk |
           TIM_CCMR2_OC3FE_Msk |
           TIM_CCMR2_OC3PE_Msk |
           TIM_CCMR2_OC3M_Msk |
           TIM_CCMR2_OC3CE_Msk);
  val |= (LED_TIM_OC_OUTPUT |
          LED_TIM_OC_FAST |
          LED_TIM_OC_PRELOAD |
          LED_TIM_OC_PWM1 |
          LED_TIM_OC_NOCLEAR);
  TIM1->CCMR2 = val;

  val = TIM1->CCER;
  val |= TIM_CCER_CC3E;
  TIM1->CCER = val;

  val = TIM1->BDTR;
  val |= TIM_BDTR_MOE;
  TIM1->BDTR = val;

  TIM1->PSC = 0;
  TIM1->ARR = LED_TIM_ARR;
  TIM1->CCR3 = LED_TIM_OC_ONE;
}

void led_start() {
  TIM1->CR1 |= TIM_CR1_CEN;
}

void led_set_pwm_zero() {
  TIM1->CCR3 = LED_TIM_OC_ZERO;
}

void led_set_pwm_one() {
  TIM1->CCR3 = LED_TIM_OC_ONE;
}



