/**
 * main.c - Giam sat tieng on STM32F103C8T6 - StdPeriph, KHONG dung RTOS
 *
 * Kieu super-loop + ngat:
 *   - Ngat DMA1_Channel1 (micro_max): dat co khi du 1 khoi mau (100 ms)
 *   - SysTick 1 ms: bo dem thoi gian he thong (g_ms) cho quet nut / timeout
 *   - Vong lap chinh: lay khoi mau -> tinh dB -> so nguong/LED -> LCD -> UART,
 *                     quet nut SET moi 10 ms va xu ly cai dat nguong
 *
 * 4 module: gpio (LED + nut), uart, lcd_i2c, micro_max
 *
 * Project: startup_stm32f10x_md.s + system_stm32f10x.c (72 MHz, HSE 8 MHz),
 *          define USE_STDPERIPH_DRIVER, STM32F10X_MD, link them -lm.
 */

#include "stm32f10x.h"
#include <stdio.h>
#include <stdint.h>

#include "gpio.h"
#include "uart.h"
#include "lcd_i2c.h"
#include "micro_max.h"

/* ---- Tham so ung dung ---- */
#define THRESHOLD_DEFAULT_DB   70U
#define THRESHOLD_MIN_DB       40U
#define THRESHOLD_MAX_DB       110U
#define ALARM_HYSTERESIS_DB    2.0f      /* tat bao dong khi dB < nguong - 2 */
#define SET_TIMEOUT_MS         5000U     /* khong thao tac 5 s thi thoat che do SET */

typedef enum { UI_RUN = 0, UI_SET } ui_mode_t;

/* ---- Thoi gian he thong (SysTick 1 ms) ---- */
static volatile uint32_t g_ms;

void SysTick_Handler(void)
{
    g_ms++;
}

static uint32_t millis(void)
{
    return g_ms;                         /* doc 32 bit tren Cortex-M3 la nguyen tu */
}

/* Loi phan cung (LCD khong phan hoi...): chop LED lien tuc */
static void Error_Handler(void)
{
    uint32_t t = millis();

    for (;;) {
        if ((millis() - t) >= 100U) {
            t = millis();
            led_alarm_toggle();
        }
    }
}

/* ---- Man hinh LCD ---- */
static void draw_run_screen(float db, uint8_t thr, uint8_t alarm)
{
    char line[24];
    int  db10 = (int)(db * 10.0f + 0.5f);

    snprintf(line, sizeof(line), "Noise:%3d.%d dB", db10 / 10, db10 % 10);
    lcd_print_line(0, line);
    snprintf(line, sizeof(line), "Thr:%3u %s", (unsigned)thr, alarm ? "ALARM!" : "NORMAL");
    lcd_print_line(1, line);
}

static void draw_set_screen(uint8_t thr)
{
    char line[24];

    lcd_print_line(0, "SET THRESHOLD");
    snprintf(line, sizeof(line), "Thr: %3u dB", (unsigned)thr);
    lcd_print_line(1, line);
}

/* ---------------- main ---------------- */
int main(void)
{
    ui_mode_t mode      = UI_RUN;
    float     db        = 0.0f;
    uint8_t   thr       = THRESHOLD_DEFAULT_DB;
    uint8_t   alarm     = 0;
    uint32_t  set_tick  = 0;
    uint32_t  last_scan = 0;

    /* SystemInit() da chay trong startup (72 MHz) */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SysTick_Config(SystemCoreClock / 1000U);          /* ngat moi 1 ms */

    uart_init();                                      /* USART1 */
    uart_send_string("Noise monitor ready\r\n");
    gpio_init();                                      /* LED + nut SET */
    mic_init();                                       /* TIM3 + ADC1 + DMA1 (chua chay) */

    if (lcd_init() != 0) {
        Error_Handler();                              /* kiem tra dia chi I2C va day noi */
    }
    draw_run_screen(db, thr, alarm);

    mic_start();

    for (;;) {
        const uint16_t *blk;
        uint8_t         new_data = 0;
        uint8_t         redraw   = 0;
        uint32_t        now      = millis();

        /* 1. Co khoi mau moi: tinh dB */
        if (mic_get_block(&blk)) {
            db = mic_compute_db(blk, MIC_BLOCK_SAMPLES);   /* khu DC, RMS, dB */
            new_data = 1;
        }

        /* 2. Quet nut SET moi 10 ms va xu ly cai dat nguong */
        if ((now - last_scan) >= BTN_SCAN_MS) {
            button_event_t ev;

            last_scan = now;
            ev = button_update(now);

            if (mode == UI_RUN) {
                if (ev != BTN_EVENT_NONE) {           /* nhan (ngan hoac giu) -> vao che do SET */
                    mode     = UI_SET;
                    set_tick = now;
                    redraw   = 1;
                }
            } else { /* UI_SET */
                if (ev == BTN_EVENT_SHORT) {          /* tang 1 dB, het khoang thi quay ve min */
                    thr = (thr >= THRESHOLD_MAX_DB) ? THRESHOLD_MIN_DB : (uint8_t)(thr + 1U);
                    set_tick = now;
                    redraw   = 1;
                } else if (ev == BTN_EVENT_LONG || (now - set_tick) >= SET_TIMEOUT_MS) {
                    mode   = UI_RUN;                  /* xac nhan nguong, ve man hinh chinh */
                    redraw = 1;
                }
            }
        }

        /* 3. State machine canh bao: Normal <-> Alarm, co hysteresis */
        if (!alarm && db >= (float)thr) {
            alarm = 1;
        } else if (alarm && db < ((float)thr - ALARM_HYSTERESIS_DB)) {
            alarm = 0;
        }
        led_alarm_set(alarm);

        /* 4. Cap nhat LCD */
        if (mode == UI_RUN) {
            if (new_data || redraw) {
                draw_run_screen(db, thr, alarm);
            }
        } else if (redraw) {
            draw_set_screen(thr);
        }

        /* 5. Gui 1 dong text len Hercules moi khi co du lieu moi */
        if (new_data) {
            uart_print_status((int)(db * 10.0f + 0.5f), thr, alarm);
        }
    }
}
