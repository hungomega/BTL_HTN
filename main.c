
#include "stm32f10x.h"
#include "hal_gpio.h"
#include "hal_uart.h"

static volatile uint8_t g_btn_flag = 0;
static volatile uint32_t g_tick_ms = 0;

void SysTick_Handler(void)
{
    g_tick_ms++;
}

/* Callback từ EXTI */
static void on_button(void)
{
    g_btn_flag = 1;
}

/* Hiển thị 4 mức LED */
static void show_level(uint8_t level)
{
    hal_gpio_write(LED_GREEN,  level >= 1);
    hal_gpio_write(LED_YELLOW, level >= 2);
    hal_gpio_write(LED_RED,    level >= 3);
}

int main(void)
{
    uint8_t level = 0;
    uint32_t last_press = 0;
    uint32_t last_uart_send = 0;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* SysTick 1ms */
    SysTick_Config(SystemCoreClock / 1000);

    /* Khởi tạo GPIO */
    hal_gpio_init();

    /* Khởi tạo UART */
    hal_uart_init();
    hal_uart_dma_init();

    hal_gpio_set_button_callback(on_button);

    show_level(level);

    while (1)
    {
        /* Xử lý nút nhấn - GIỮ NGUYÊN GPIO */
        if (g_btn_flag)
        {
            g_btn_flag = 0;

            /* Chống rung 50ms */
            if ((g_tick_ms - last_press) >= 50)
            {
                if (hal_gpio_read() == BUTTON_PRESSED)
                {
                    last_press = g_tick_ms;

                    /* Tăng mức LED */
                    level = (level + 1) % 4;

                    show_level(level);
                }
            }
        }

        /* Gửi UART mỗi 1 giây */
      if ((g_tick_ms - last_uart_send) >= 1000)
        {
            if (hal_uart_send_dma("STM32 USART1 DMA OK!\r\n"))
            {
                last_uart_send = g_tick_ms;
            }
        }
    }
}
