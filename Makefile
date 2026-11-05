# Makefile build STM32F103C8T6 bang arm-none-eabi-gcc (chay trong Git Bash)
TARGET  := btl_htn
BUILD   := build

CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size

# --- FreeRTOS (tu dong them vao khi da chep nhan vao middlewares/FreeRTOS) ---
RTOS_DIR  := middlewares/FreeRTOS
RTOS_PORT := $(RTOS_DIR)/portable/GCC/ARM_CM3
RTOS_SRC  := $(wildcard $(RTOS_DIR)/*.c) \
             $(wildcard $(RTOS_PORT)/port.c) \
             $(wildcard $(RTOS_DIR)/portable/MemMang/heap_4.c)
RTOS_INC  := -I$(RTOS_DIR)/include -I$(RTOS_PORT)

# --- Ma nguon ---
STDPERIPH := misc stm32f10x_rcc stm32f10x_gpio stm32f10x_adc stm32f10x_dma \
             stm32f10x_tim stm32f10x_usart stm32f10x_i2c stm32f10x_exti stm32f10x_spi

C_SRCS := main.c \
          $(wildcard hal/src/*.c) \
          $(wildcard app/src/*.c) \
          bsp/cmsis/system_stm32f10x.c \
          $(addprefix bsp/stdperiph/src/,$(addsuffix .c,$(STDPERIPH))) \
          $(RTOS_SRC)
S_SRCS := bsp/startup_stm32f10x_md.s

OBJS := $(addprefix $(BUILD)/,$(C_SRCS:.c=.o) $(S_SRCS:.s=.o))

# --- Co bien dich ---
CPU    := -mcpu=cortex-m3 -mthumb
DEFS   := -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER
INCS   := -Ibsp -Ibsp/cmsis -Ibsp/stdperiph/inc -Ihal/inc -Iapp/inc $(RTOS_INC)
CFLAGS := $(CPU) $(DEFS) $(INCS) -std=gnu11 -Os -g3 -Wall \
          -ffunction-sections -fdata-sections -MMD -MP
LDFLAGS := $(CPU) -T bsp/STM32F103C8Tx_FLASH.ld -nostartfiles \
           --specs=nano.specs --specs=nosys.specs \
           -Wl,--gc-sections -Wl,-Map=$(BUILD)/$(TARGET).map

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).bin $(BUILD)/$(TARGET).hex
	@$(SIZE) $(BUILD)/$(TARGET).elf

$(BUILD)/$(TARGET).elf: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(BUILD)/%.bin: $(BUILD)/%.elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/%.hex: $(BUILD)/%.elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	$(CC) $(CPU) -c $< -o $@

# Nap bang ST-Link (stlink-tools): make flash
flash: $(BUILD)/$(TARGET).bin
	st-flash --reset write $< 0x08000000

clean:
	rm -rf $(BUILD)

-include $(OBJS:.o=.d)

.PHONY: all flash clean
