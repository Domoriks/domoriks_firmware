/* Microsecond timer helper using TIM2. */

#include "timing_us.h"
#include "stm32g0xx_hal.h"

static TIM_HandleTypeDef htim_us;

void timing_us_init(void)
{
    /* Enable TIM2 clock and configure for 1 MHz tick (1 us) */
    __HAL_RCC_TIM2_CLK_ENABLE();

    htim_us.Instance = TIM2;

    uint32_t pclk = HAL_RCC_GetPCLK1Freq();
    uint32_t prescaler = (pclk / 1000000UL);
    if (prescaler == 0) prescaler = 1;

    htim_us.Init.Prescaler = prescaler - 1U;
    htim_us.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim_us.Init.Period = 0xFFFFFFFFU;
    htim_us.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;

    if (HAL_TIM_Base_Init(&htim_us) != HAL_OK)
    {
        /* Initialization error - fail silently; caller may check behavior */
    }

    /* Start timer in free-running mode */
    HAL_TIM_Base_Start(&htim_us);
}

uint32_t timing_us_now(void)
{
    return (uint32_t)__HAL_TIM_GET_COUNTER(&htim_us);
}

void timing_us_delay_us(uint32_t us)
{
    uint32_t start = timing_us_now();
    while ((uint32_t)(timing_us_now() - start) < us) {
        __NOP();
    }
}
