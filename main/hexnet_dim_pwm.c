#include "hexnet_dim_pwm.h"

#include <math.h>

#include "driver/ledc.h"
#include "hexnet_log.h"
#define LOG_LOCAL_LEVEL HEXNET_LOG_LEVEL_OTHER
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hexnet_canbus.h"
#include "hexnet_io_map.h"

#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES   LEDC_TIMER_10_BIT
#define LEDC_FREQ_HZ    5000
#define MAX_DUTY        ((1 << 10) - 1)
#define DIM_CHANNEL_COUNT 8

static const gpio_num_t s_dim_gpios[DIM_CHANNEL_COUNT] = {
    HEXNET_IO_DIM_PWM_GPIO_1,
    HEXNET_IO_DIM_PWM_GPIO_2,
    HEXNET_IO_DIM_PWM_GPIO_3,
    HEXNET_IO_DIM_PWM_GPIO_4,
    HEXNET_IO_DIM_PWM_GPIO_5,
    HEXNET_IO_DIM_PWM_GPIO_6,
    HEXNET_IO_DIM_PWM_GPIO_7,
    HEXNET_IO_DIM_PWM_GPIO_8,
};

static void ledc_init_channel(ledc_channel_t ch, gpio_num_t gpio)
{
    ledc_channel_config_t cfg = {
        .speed_mode = LEDC_MODE,
        .channel = ch,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = gpio,
        .duty = 0,
        .hpoint = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&cfg));
}

static uint32_t scale_brightness(uint32_t pct)
{
    if (pct == 0) {
        return 0;
    }
    const float gamma = 2.0f;
    float norm = (float)pct / 100.0f;
    return (uint32_t)(powf(norm, gamma) * MAX_DUTY + 0.5f);
}

void hexnet_dim_pwm_task(void *arg)
{
    (void)arg;

    ledc_timer_config_t timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQ_HZ,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    for (int i = 0; i < DIM_CHANNEL_COUNT; ++i) {
        ledc_init_channel((ledc_channel_t)i, s_dim_gpios[i]);
    }

    while (1) {
        /* Tek kaynak: set_dimmable_output() — BLE, MQTT ve CAN ayni buffer. */
        for (int i = 0; i < DIM_CHANNEL_COUNT; ++i) {
            uint32_t duty = scale_brightness(get_dimmable_output((uint8_t)i));
            ledc_set_duty(LEDC_MODE, (ledc_channel_t)i, duty);
            ledc_update_duty(LEDC_MODE, (ledc_channel_t)i);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
