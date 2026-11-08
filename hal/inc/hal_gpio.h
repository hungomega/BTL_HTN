#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>

/* LED: PB0 = xanh, PB1 = vang, PB5 = do (muc cao = sang) */
#define LED_GREEN       0
#define LED_YELLOW      1
#define LED_RED         2
#define LED_COUNT       3

#define GPIO_LOW        0
#define GPIO_HIGH       1

/* Nut nhan PB8, pull-up noi: nhan = muc thap */
#define BUTTON_PRESSED  0
#define BUTTON_RELEASED 1

typedef void (*hal_gpio_cb_t)(void);

void    hal_gpio_init(void);
void    hal_gpio_write(uint8_t led, uint8_t state);
void    hal_gpio_toggle(uint8_t led);
uint8_t hal_gpio_read(void);

/* Ham cb duoc goi trong ngat EXTI khi nut duoc nhan (canh xuong).
   Chi nen dat co/gui notification, khong xu ly nang trong cb. */
void    hal_gpio_set_button_callback(hal_gpio_cb_t cb);

#endif /* HAL_GPIO_H */