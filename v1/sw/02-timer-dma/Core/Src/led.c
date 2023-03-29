
#include <stm32g0xx_hal.h>
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

#define LED_DMA_VHI_PRIO (DMA_CCR_PL_0 | DMA_CCR_PL_1)
#define LED_DMA_MSIZE_16 (DMA_CCR_MSIZE_0)
#define LED_DMA_PSIZE_16 (DMA_CCR_PSIZE_0)
#define LED_DMA_MEM_INC  (DMA_CCR_MINC)
#define LED_DMA_CIRCULAR (DMA_CCR_CIRC)
#define LED_DMA_MEM2TIM  (DMA_CCR_DIR)

#define LED_DMAMUX_REQID (25 << DMAMUX_CxCR_DMAREQ_ID_Pos)

#define LED_DMA_BUFFER_SIZE 48


static uint16_t g_led_dma_buffer[LED_DMA_BUFFER_SIZE] = {
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
  LED_TIM_OC_ONE,
  LED_TIM_OC_ZERO,
};


static void rcc_init(void);
static void gpio_init(void);
static void tim_init(void);
static void dma_init(void);


static void rcc_init() {
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_TIM1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();
}

static void gpio_init() {
  uint32_t val;

  val = GPIOA->MODER;
  val &= ~GPIO_MODER_MODE10_Msk;
  val |= LED_GPIO_ALT_FUNC;
  GPIOA->MODER = val;

  val = GPIOA->AFR[1];
  val &= ~GPIO_AFRH_AFSEL10_Msk;
  val |= LED_GPIO_PA10_AF2;
  GPIOA->AFR[1] = val;
}

static void tim_init() {
  uint32_t val;

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

  TIM1->CCER |= TIM_CCER_CC3E;

  TIM1->BDTR |= TIM_BDTR_MOE;

  TIM1->DIER |= TIM_DIER_UDE;

  TIM1->PSC = 0;
  TIM1->ARR = LED_TIM_ARR;
  TIM1->CCR3 = LED_TIM_OC_ONE;
}

static void dma_init() {
  DMA1_Channel1->CCR = (LED_DMA_VHI_PRIO |
                        LED_DMA_MSIZE_16 |
                        LED_DMA_PSIZE_16 |
                        LED_DMA_MEM_INC |
                        LED_DMA_CIRCULAR |
                        LED_DMA_MEM2TIM);
  DMA1_Channel1->CNDTR = LED_DMA_BUFFER_SIZE;
  DMA1_Channel1->CPAR = (uint32_t)&(TIM1->CCR3);
  DMA1_Channel1->CMAR = (uint32_t)g_led_dma_buffer;

  DMAMUX1_Channel0->CCR = LED_DMAMUX_REQID;

  DMA1_Channel1->CCR |= DMA_CCR_EN;
}

void led_init() {
  rcc_init();
  gpio_init();
  tim_init();
  dma_init();
  // TODO nvic_init
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



