#include "hal_gpio.h"
#include "stm32f10x.h"

static const uint16_t s_led_pin[LED_COUNT] = { GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_5 };
static hal_gpio_cb_t s_button_cb = 0;

void hal_gpio_init(void)
{
    GPIO_InitTypeDef gpio;
    EXTI_InitTypeDef exti;
    NVIC_InitTypeDef nvic;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    /* 3 LED: output push-pull, tat het luc dau */
    gpio.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5;
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &gpio);
    GPIO_ResetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_5);

    /* Nut nhan PB8: input pull-up */
    gpio.GPIO_Pin  = GPIO_Pin_8;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &gpio);

    /* EXTI line 8, canh xuong (luc nhan) */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);
    exti.EXTI_Line    = EXTI_Line8;
    exti.EXTI_Mode    = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Falling;
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);

    /* Uu tien 10: du thap de dung duoc API ...FromISR cua FreeRTOS sau nay */
    nvic.NVIC_IRQChannel                   = EXTI9_5_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 10;
    nvic.NVIC_IRQChannelSubPriority        = 0;
    nvic.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&nvic);
}

void hal_gpio_write(uint8_t led, uint8_t state)
{
    if (led >= LED_COUNT) return;
    if (state == GPIO_HIGH)     GPIO_SetBits(GPIOB, s_led_pin[led]);
    else if (state == GPIO_LOW) GPIO_ResetBits(GPIOB, s_led_pin[led]);
}

void hal_gpio_toggle(uint8_t led)
{
    if (led >= LED_COUNT) return;
    GPIO_WriteBit(GPIOB, s_led_pin[led],
                  (BitAction)!GPIO_ReadOutputDataBit(GPIOB, s_led_pin[led]));
}

uint8_t hal_gpio_read(void)
{
    return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8) ? BUTTON_RELEASED : BUTTON_PRESSED;
}

void hal_gpio_set_button_callback(hal_gpio_cb_t cb)
{
    s_button_cb = cb;
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line8) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line8);
        if (s_button_cb) s_button_cb();
    }
}