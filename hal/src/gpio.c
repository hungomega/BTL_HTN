#include "gpio.h"

/* ================= Nut nhan ================= */
typedef enum {
    BTN_ST_IDLE = 0, // chờ người dùng bấm nút
    BTN_ST_DEBOUNCE, // chờ 30 ms để loại bỏ hiện tượng dội nút
    BTN_ST_PRESSED, // Theo dõi thời gian nhấn để phân biệt nhấn nhanh và nhấn giữ
    BTN_ST_WAIT_RELEASE // Đã nhận sự kiện nhấn giữ , chờ người dùng thả nút
} btn_state_t;

static btn_state_t s_state;
static uint32_t    s_t0;

/* ================= Khoi tao ================= */
void gpio_init(void)
{
    GPIO_InitTypeDef gpio;
    
    // Bật clock GPIOB
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* LED PB0: output push-pull, tat luc khoi dong */
    GPIO_ResetBits(LED_ALARM_PORT, LED_ALARM_PIN);
    gpio.GPIO_Pin   = LED_ALARM_PIN; // use chân PIN 0
    gpio.GPIO_Mode  = GPIO_Mode_Out_PP; // cấu hình output push pull
    gpio.GPIO_Speed = GPIO_Speed_2MHz; // tốc độ speed 2Mhz
 
 
    // Cấu hình Button: truyền port B vào
    GPIO_Init(LED_ALARM_PORT, &gpio);

    /* Nut PB8: input keo len */
    gpio.GPIO_Pin  = BTN_PIN; // use Pin 8
    gpio.GPIO_Mode = GPIO_Mode_IPU; // input pull up : high: 1 không nhấn , low: 0 nhấn
    GPIO_Init(BTN_PORT, &gpio);

    s_state = BTN_ST_IDLE; // s_state: Biến lưu trạng thái hiện tại của nút nhấn , BTN_ST_IDLE: Trạng thái nghỉ, tức là chưa có thao tác nhấn nút nào đang được xử lý . Đưa State Machine về trạng thái nghỉ
    s_t0    = 0; // 	s_t0 là biến lưu thời điểm bắt đầu nhấn nút,
}

/* ================= LED ================= */
void led_alarm_set(uint8_t on)
{
    if (on) {
        GPIO_SetBits(LED_ALARM_PORT, LED_ALARM_PIN); // Xuất HIGH → LED sáng.
    } else {
        GPIO_ResetBits(LED_ALARM_PORT, LED_ALARM_PIN); // Xuất LOW → LED tắt
    }
}
// hàm đảo led
void led_alarm_toggle(void)
{
    if (GPIO_ReadOutputDataBit(LED_ALARM_PORT, LED_ALARM_PIN) != Bit_RESET) {
        GPIO_ResetBits(LED_ALARM_PORT, LED_ALARM_PIN);
    } else {
        GPIO_SetBits(LED_ALARM_PORT, LED_ALARM_PIN);
    }
}

/* ================= Nut: state machine chong rung ================= */
button_event_t button_update(uint32_t now)
{
    uint8_t pressed = (GPIO_ReadInputDataBit(BTN_PORT, BTN_PIN) == BTN_ACTIVE_LEVEL) ? 1U : 0U;

    switch (s_state) {
        // trạng thái nghỉ , chờ người dùng nhấn nút . Khi phát hiện nhấn , lưu thời điểm bắt đầu vào s_t0
    case BTN_ST_IDLE:
        if (pressed) {
            s_t0    = now; // lưu thời điểm nhấn hiện tại
            s_state = BTN_ST_DEBOUNCE; // chuyển sang trại thái chống runng
        }
        break;

    /* Trang thai 2: Kiem tra chong rung nut nhan */
    case BTN_ST_DEBOUNCE:
        if (!pressed) {
            s_state = BTN_ST_IDLE;  /* Nut bi nha som, huy qua trinh nhan */
        } else if ((*now - s_t0) >= BTN_DEBOUNCE_MS) {
            s_state = BTN_ST_PRESSED; /* Nhan on dinh du thoi gian, xac nhan nhan hop le */
        }
        break;

    /* Trang thai 3: Nut da duoc nhan hop le */
    case BTN_ST_PRESSED:
        if (!pressed) {
            s_state = BTN_ST_IDLE;  /* Nguoi dung tha nut */
            return BTN_EVENT_SHORT; /* Phat hien su kien nhan ngan */
        }

        if ((*now - s_t0) >= BTN_LONG_PRESS_MS) {
            s_state = BTN_ST_WAIT_RELEASE; /* Dat nguong nhan dai, cho tha nut */
            return BTN_EVENT_LONG;         /* Phat hien su kien nhan dai */
        }
        break;

    /* Trang thai 4: Da nhan dai, cho nguoi dung tha nut */
    case BTN_ST_WAIT_RELEASE:
        if (!pressed) {
            s_state = BTN_ST_IDLE; /* Tha nut, cho phep nhan lan tiep theo */
        }
        break;

    /* Trang thai khong hop le: Dua state machine ve trang thai ban dau */
    default:
        s_state = BTN_ST_IDLE;
        break;
    }

    /* Khong phat hien su kien nhan nut */
    return BTN_EVENT_NONE;
}
