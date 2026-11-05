/*
 * Chuong trinh thu toolchain: nhay LED PC13 (LED tren Blue Pill, muc thap la sang).
 * Sau nay main.c se thanh: khoi tao BSP/HAL, tao task, vTaskStartScheduler().
 */
#include "stm32f10x.h"

static volatile uint32_t g_tick_ms = 0;

/* TAM THOI: khi tich hop FreeRTOS thi xoa ham nay (FreeRTOS tu quan ly SysTick). */
void SysTick_Handler(void)
{
    g_tick_ms++;
}

static void delay_ms(uint32_t ms)
{
    uint32_t start = g_tick_ms;
    while ((g_tick_ms - start) < ms) { }
}

int main(void)
{
    GPIO_InitTypeDef gpio;

    SysTick_Config(SystemCoreClock / 1000);      /* ngat moi 1 ms */

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    gpio.GPIO_Pin   = GPIO_Pin_13;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &gpio);

    for (;;)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);       /* LED sang */
        delay_ms(500);
        GPIO_SetBits(GPIOC, GPIO_Pin_13);         /* LED tat */
        delay_ms(500);
    }
}
