
#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>

/* ================= GPIO DEFINITIONS ================= */

/* LED identifiers */
#define LED_GREEN       0
#define LED_YELLOW      1
#define LED_RED         2

/* GPIO output states */
#define GPIO_LOW        0
#define GPIO_HIGH       1

/* Button states */
#define BUTTON_PRESSED  0
#define BUTTON_RELEASED 1

/* ================= FUNCTION PROTOTYPES ================= */

/* Initialize GPIO pins */
void hal_gpio_init(void);

/* Control LED */
void hal_gpio_write(uint8_t led, uint8_t state);

/* Read button state */
uint8_t hal_gpio_read(void);

#endif /* HAL_GPIO_H */
