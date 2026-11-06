#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_i2c.h"
#include <stdio.h>

// Ð?a ch? I2C c?a module PCF8574 (thu?ng là 0x27 ho?c 0x3F, d?ch trái 1 bit)
#define LCD_I2C_ADDR    (0x27 << 1) 

// Ð?nh nghia các chân tín hi?u k?t n?i trên chip m? r?ng PCF8574 cho LCD 1602
#define LCD_RS_PIN      0x01  // P0 - Register Select
#define LCD_RW_PIN      0x02  // P1 - Read/Write
#define LCD_EN_PIN      0x04  // P2 - Enable pin
#define LCD_BACKLIGHT   0x08  // P3 - Ðèn n?n LCD

/* ==================== 1. T?NG I2C1 (PB6: SCL, PB7: SDA) ==================== */

void I2C1_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef  I2C_InitStructure;

    // B?t clock cho GPIOB, AFIO và I2C1[cite: 2]
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // C?u hình chân PB6 (SCL) và PB7 (SDA) d?ng Alternate Function Open-Drain[cite: 2]
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_OD; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // C?u hình thông s? I2C1 (T?c d? chu?n 100kHz)
    I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1         = 0x00;
    I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed          = 100000; 

    I2C_Init(I2C1, &I2C_InitStructure);
    I2C_Cmd(I2C1, ENABLE);
}

void I2C_WriteBytes(uint8_t slave_addr, uint8_t *pData, uint16_t size) {
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, slave_addr, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    for(uint16_t i = 0; i < size; i++) {
        I2C_SendData(I2C1, pData[i]);
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING));
    }
    
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_GenerateSTOP(I2C1, ENABLE);
}

void I2C_WriteByte(uint8_t slave_addr, uint8_t data) {
    I2C_WriteBytes(slave_addr, &data, 1);
}


/* ==================== 2. T?NG LCD 1602 I2C ==================== */

static void LCD_Write_I2C(uint8_t data) {
    I2C_WriteByte(LCD_I2C_ADDR, data);
}

static void LCD_Send_Nibble(uint8_t nibble, uint8_t control) {
    uint8_t data = nibble | control | LCD_BACKLIGHT;
    LCD_Write_I2C(data | LCD_EN_PIN); // B?t chân EN
    for(volatile uint32_t i = 0; i < 150; i++); 
    LCD_Write_I2C(data & ~LCD_EN_PIN); // T?t chân EN d? ch?t d? li?u
}

static void LCD_Send_Cmd_Data(uint8_t value, uint8_t control) {
    uint8_t high_nibble = value & 0xF0;
    uint8_t low_nibble = (value << 4) & 0xF0;
    LCD_Send_Nibble(high_nibble, control);
    LCD_Send_Nibble(low_nibble, control);
}

void LCD_SendCommand(uint8_t cmd) {
    LCD_Send_Cmd_Data(cmd, 0); // RS = 0 (L?nh)
}

void LCD_SendData(uint8_t data) {
    LCD_Send_Cmd_Data(data, LCD_RS_PIN); // RS = 1 (D? li?u)
}

void LCD_Init(void) {
    I2C1_Init(); // Kh?i t?o ph?n c?ng I2C1 tru?c
    for(volatile uint32_t i = 0; i < 50000; i++); 

    LCD_Send_Nibble(0x30, 0);
    for(volatile uint32_t i = 0; i < 10000; i++);
    LCD_Send_Nibble(0x30, 0);
    for(volatile uint32_t i = 0; i < 1000; i++);
    LCD_Send_Nibble(0x30, 0);
    LCD_Send_Nibble(0x20, 0); // Ch? d? 4-bit

    LCD_SendCommand(0x28); // 2 dòng, font 5x8
    LCD_SendCommand(0x0C); // B?t hi?n th?, t?t con tr?
    LCD_SendCommand(0x06); // Tang con tr? t? d?ng
    LCD_SendCommand(0x01); // Xóa màn hình
    for(volatile uint32_t i = 0; i < 5000; i++);
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t address = (row == 0) ? 0x80 : 0xC0;
    address += col;
    LCD_SendCommand(address);
}

void LCD_Print(const char *str) {
    while (*str) {
        LCD_SendData(*str++);
    }
}

// Hàm hi?n th? m?c ?n (dB) và tr?ng thái theo yêu c?u d? cuong bài t?p l?n[cite: 1, 2]
void LCD_Display_NoiseData(float db_value, float threshold, const char *status_str) {
    char buffer[17];

    LCD_SetCursor(0, 0);
    snprintf(buffer, sizeof(buffer), "dB:%.1f Th:%.1f", db_value, threshold);
    LCD_Print(buffer);

    LCD_SetCursor(1, 0);
    snprintf(buffer, sizeof(buffer), "Status: %-8s", status_str);
    LCD_Print(buffer);
}