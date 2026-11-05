
#include "hal_uart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_dma.h"
#include <string.h>

/* ================= UART1 ================= */

void hal_uart_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* Cap clock GPIOA va USART1 */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,
        ENABLE
    );

    /* PA9 - TX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA10 - RX */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Cau hinh USART1 */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode =
        USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

/* Gui mot ky tu bang polling */
void hal_uart_send_char(char data)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);

    USART_SendData(USART1, (uint16_t)data);
}

/* Gui chuoi bang polling */
void hal_uart_send_string(const char *str)
{
    if (str == 0)
        return;

    while (*str)
    {
        hal_uart_send_char(*str++);
    }
}

/* ================= UART DMA ================= */

/* Khoi tao DMA1 Channel 4 cho USART1_TX */
void hal_uart_dma_init(void)
{
    DMA_InitTypeDef DMA_InitStructure;

    /* Bat clock DMA1 */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* Reset DMA Channel 4 */
    DMA_DeInit(DMA1_Channel4);
    DMA_ClearFlag(DMA1_FLAG_GL4);

    DMA_InitStructure.DMA_PeripheralBaseAddr =
        (uint32_t)&USART1->DR;

    DMA_InitStructure.DMA_MemoryBaseAddr = 0;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0;

    DMA_InitStructure.DMA_PeripheralInc =
        DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc =
        DMA_MemoryInc_Enable;

    DMA_InitStructure.DMA_PeripheralDataSize =
        DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize =
        DMA_MemoryDataSize_Byte;

    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel4, &DMA_InitStructure);

    /* Cho phep USART1 yeu cau DMA khi truyen */
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}

/* Kiem tra DMA dang truyen */
uint8_t hal_uart_dma_busy(void)
{
    /* Neu da truyen xong */
    if (DMA_GetFlagStatus(DMA1_FLAG_TC4) == SET)
    {
        DMA_Cmd(DMA1_Channel4, DISABLE);
        DMA_ClearFlag(DMA1_FLAG_GL4);
        return 0;
    }

    /* Neu DMA van dang hoat dong */
    if (DMA1_Channel4->CCR & DMA_CCR1_EN)
        return 1;

    return 0;
}

/* Gui chuoi bang DMA1 Channel 4 */
uint8_t hal_uart_send_dma(const char *str)
{
    uint16_t length;

    if (str == 0 || *str == '\0')
        return 0;

    /* Kiem tra DMA dang ban */
    if (hal_uart_dma_busy())
        return 0;

    length = (uint16_t)strlen(str);

    if (length == 0)
        return 0;

    /* Tat DMA truoc khi nap du lieu moi */
    DMA_Cmd(DMA1_Channel4, DISABLE);

    /* Xoa co cu */
    DMA_ClearFlag(DMA1_FLAG_GL4);

    /* Nap dia chi bo nho va so byte */
    DMA1_Channel4->CMAR = (uint32_t)str;
    DMA_SetCurrDataCounter(DMA1_Channel4, length);

    /* Bat DMA truyen */
    DMA_Cmd(DMA1_Channel4, ENABLE);

    return 1;
}
