
#include <stm32g0xx_hal.h>
#include "led.h"

#define LED_TIM_ARR     79
#define LED_TIM_OC_ZERO 16
#define LED_TIM_OC_ONE  38
#define LED_TIM_OC_LOW  0
#define LED_TIM_OC_HIGH 0xffff

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
#define LED_DMA_MEM2TIM  (DMA_CCR_DIR)
#define LED_DMA_FULL_IRQ (DMA_CCR_TCIE)

#define LED_DMAMUX_REQID (25 << DMAMUX_CxCR_DMAREQ_ID_Pos)

#define BITS_PER_COLOR 8
#define BITS_PER_LED (BITS_PER_COLOR*NUM_COLORS)
#define LED_DMA_BUFFER_SIZE (BITS_PER_LED*NUM_LEDS + 1)


typedef enum {
  BLUE = 0,
  RED,
  GREEN,
  NUM_COLORS,
} Color;

typedef struct {
  uint8_t color[NUM_COLORS];
} Led;


static Led g_led_strip[NUM_LEDS];


static uint16_t led_dma_buffer[LED_DMA_BUFFER_SIZE] = { LED_TIM_OC_LOW };


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
  TIM1->CCR3 = LED_TIM_OC_LOW;
}

static void dma_init() {
  DMA1_Channel1->CCR = (LED_DMA_VHI_PRIO |
                        LED_DMA_MSIZE_16 |
                        LED_DMA_PSIZE_16 |
                        LED_DMA_MEM_INC |
                        LED_DMA_MEM2TIM |
                        LED_DMA_FULL_IRQ);
  DMA1_Channel1->CNDTR = LED_DMA_BUFFER_SIZE;
  DMA1_Channel1->CPAR = (uint32_t)&(TIM1->CCR3);
  DMA1_Channel1->CMAR = (uint32_t)led_dma_buffer;

  DMAMUX1_Channel0->CCR = LED_DMAMUX_REQID;

  DMA1_Channel1->CCR |= DMA_CCR_EN;
}

static void nvic_init() {
  NVIC->ISER[0] = (1 << DMA1_Channel1_IRQn);
}

void led_init() {
  rcc_init();
  gpio_init();
  tim_init();
  dma_init();
  nvic_init();
}

void led_set_color(int pos, uint8_t red, uint8_t green, uint8_t blue) {
  if (0 > pos || NUM_LEDS <= pos) {
    return;
  }

  g_led_strip[pos].color[RED]   = red;
  g_led_strip[pos].color[GREEN] = green;
  g_led_strip[pos].color[BLUE]  = blue;
}

void led_update_strip(void) {
  uint8_t color;
  int i, j, k, pos = 0;

  for (i = 0; i < NUM_LEDS; i++) {
    for (j = 0; j < NUM_COLORS; j++) {
      color = g_led_strip[i].color[j];
      for (k = BITS_PER_COLOR-1; k >= 0; k--) {
        led_dma_buffer[pos++] = ((1 << k) & color) ? LED_TIM_OC_ONE : LED_TIM_OC_ZERO;
      }
    }
  }
  led_dma_buffer[LED_DMA_BUFFER_SIZE-1] = LED_TIM_OC_LOW;

  DMA1_Channel1->CNDTR = LED_DMA_BUFFER_SIZE;
  DMA1_Channel1->CMAR = (uint32_t)led_dma_buffer;
  DMA1_Channel1->CCR |= DMA_CCR_EN;
  TIM1->CCR3 = LED_TIM_OC_LOW;
  TIM1->CR1 |= TIM_CR1_CEN;
}

void DMA1_Channel1_IRQHandler(void) {
  TIM1->CR1 &= ~TIM_CR1_CEN;
  DMA1_Channel1->CCR &= ~DMA_CCR_EN;

  if ((DMA_ISR_GIF1 | DMA_ISR_HTIF1 | DMA_ISR_TCIF1) == DMA1->ISR) {
    DMA1->IFCR = DMA_IFCR_CGIF1;
    return;
  }

  while (1);
}

