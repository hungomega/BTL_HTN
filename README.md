# BTL Hệ thống nhúng: Giám sát tiếng ồn môi trường (STM32F103C8T6 + FreeRTOS)

## Build
Chạy trong Git Bash, đứng ở thư mục gốc dự án:

    make            # biên dịch, ra build/btl_htn.elf, .bin, .hex
    make flash      # nạp bằng ST-Link (cần stlink-tools: st-flash)
    make clean

## Cấu trúc
- `bsp/`: startup, linker script, CMSIS, Standard Peripheral Library
- `hal/`: driver ngoại vi (inc/, src/)
- `middlewares/FreeRTOS/`: nhân FreeRTOS (Makefile tự nhận khi có file)
- `app/`: các task FreeRTOS và IPC
- `main.c`: khởi tạo, tạo task, vTaskStartScheduler()
