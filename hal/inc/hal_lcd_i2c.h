#ifndef __LCD_I2C_H
#define __LCD_I2C_H

#include "stm32f10x.h"

// --- T?ng I2C1 ---
void I2C1_Init(void);
void I2C_WriteBytes(uint8_t slave_addr, uint8_t *pData, uint16_t size);
void I2C_WriteByte(uint8_t slave_addr, uint8_t data);

// --- T?ng LCD 1602 I2C (PCF8574) ---
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_Init(void);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_Print(const char *str);

// --- Hàm hi?n th? chuyên d?ng cho h? th?ng do ti?ng ?n ---
void LCD_Display_NoiseData(float db_value, float threshold, const char *status_str);

#endif /* __LCD_I2C_H */
