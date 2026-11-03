
#include "hal_gpio.h"

/* ================= RCC REGISTERS ================= */

#define RCC_APB2ENR     (*(volatile uint32_t *)0x40021018)

/* ================= GPIOB REGISTERS ================= */

#define GPIOB_CRL       (*(volatile uint32_t *)0x40010C00)
#define GPIOB_CRH       (*(volatile uint32_t *)0x40010C04)
#define GPIOB_IDR       (*(volatile uint32_t *)0x40010C08)
#define GPIOB_ODR       (*(volatile uint32_t *)0x40010C0C)
#define GPIOB_BSRR      (*(volatile uint32_t *)0x40010C10)
#define GPIOB_BRR       (*(volatile uint32_t *)0x40010C14)

/* ================= PIN DEFINITIONS ================= */

#define LED_GREEN_PIN   0
#define LED_YELLOW_PIN  1
#define LED_RED_PIN     5

#define BUTTON_PIN      8

/* ================= GPIO INITIALIZATION ================= */

void hal_gpio_init(void)
{
    /* Enable GPIOB clock */
    RCC_APB2ENR |= (1U << 3);

    /* PB0: Output push-pull, 2MHz */
    GPIOB_CRL &= ~(0xFU << 0);
    GPIOB_CRL |=  (0x2U << 0);

    /* PB1: Output push-pull, 2MHz */
    GPIOB_CRL &= ~(0xFU << 4);
    GPIOB_CRL |=  (0x2U << 4);

    /* PB5: Output push-pull, 2MHz */
    GPIOB_CRL &= ~(0xFU << 20);
    GPIOB_CRL |=  (0x2U << 20);

    /* PB8: Input pull-up / pull-down */
    GPIOB_CRH &= ~(0xFU << 0);
    GPIOB_CRH |=  (0x8U << 0);

    /* Select pull-up */
    GPIOB_ODR |= (1U << BUTTON_PIN);

    /* Turn off all LEDs initially */
    GPIOB_BRR = (1U << LED_GREEN_PIN)
              | (1U << LED_YELLOW_PIN)
              | (1U << LED_RED_PIN);
}

/* ================= LED CONTROL ================= */

void hal_gpio_write(uint8_t led, uint8_t state)
{
    uint8_t pin;

    /* Select LED pin */
    switch (led)
    {
        case LED_GREEN:
            pin = LED_GREEN_PIN;
            break;

        case LED_YELLOW:
            pin = LED_YELLOW_PIN;
            break;

        case LED_RED:
            pin = LED_RED_PIN;
            break;

        default:
            return;
    }

    /* Set or reset GPIO pin */
    if (state == GPIO_HIGH)
    {
        GPIOB_BSRR = (1U << pin);
    }
    else if (state == GPIO_LOW)
    {
        GPIOB_BRR = (1U << pin);
    }
}

/* ================= BUTTON READING ================= */

uint8_t hal_gpio_read(void)
{
    /* Pull-up:
       Pressed  = LOW
       Released = HIGH
    */

    if (GPIOB_IDR & (1U << BUTTON_PIN))
    {
        return BUTTON_RELEASED;
    }

    return BUTTON_PRESSED;
}
