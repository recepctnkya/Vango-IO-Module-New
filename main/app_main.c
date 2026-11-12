/*
 * Staged bring-up entry (HEXNET_BUILD_STEP from CMake).
 * Step 1: Run LED blink + 12-relay walk on shift register.
 */
#include "hexnet_build_step.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hexnet_io_map.h"

#if HEXNET_IS_FULL
#include <inttypes.h>
#include "hexnet_log_init.h"
#define LOG_LOCAL_LEVEL ESP_LOG_ERROR
#include "esp_system.h"
#include "hexnet_app.h"
#include "hexnet_board.h"
#include "hexnet_mqtt.h"
#include "hexnet_wifi.h"
#include "hexnet_io_profile.h"
#include "hexnet_cjson_psram.h"
#include "hexnet_shift_register.h"
#include "nvs_flash.h"
#include "ota_manager.h"
#endif

static const char *TAG = "STEP";

#if !HEXNET_IS_FULL

#define LED_MS          500
#define RELAY_HOLD_MS   700
#define RELAY_GAP_MS    250

static void sr_write(uint16_t value)
{
    for (int i = 15; i >= 0; i--) {
        gpio_set_level(HEXNET_IO_SR_CLK_GPIO, 0);
        gpio_set_level(HEXNET_IO_SR_DATA_GPIO, (value >> i) & 0x01);
        gpio_set_level(HEXNET_IO_SR_CLK_GPIO, 1);
    }
    gpio_set_level(HEXNET_IO_SR_STB_GPIO, 0);
    gpio_set_level(HEXNET_IO_SR_STB_GPIO, 1);
}

static void gpio_out_init(uint64_t mask)
{
    gpio_config_t io = {
        .pin_bit_mask = mask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);
}

#if HEXNET_HAS_DIMMER
/* Step 2+ hooks added in later commits */
#endif

void app_main(void)
{
    gpio_out_init((1ULL << HEXNET_IO_RUN_LED_GPIO) | HEXNET_IO_SR_MASK);
    gpio_set_level(HEXNET_IO_RUN_LED_GPIO, 0);
    gpio_set_level(HEXNET_IO_SR_CLK_GPIO, 0);
    gpio_set_level(HEXNET_IO_SR_DATA_GPIO, 0);
    gpio_set_level(HEXNET_IO_SR_STB_GPIO, 0);
    sr_write(0);

    ESP_LOGE(TAG, "HEXNET staged boot STEP=%d (0=LED 1=relay .. 99=full)", HEXNET_BUILD_STEP);

#if HEXNET_BUILD_STEP == 0
    ESP_LOGE(TAG, "LED-ONLY gpio=%d period=%dms", (int)HEXNET_IO_RUN_LED_GPIO, LED_MS);
    bool led = false;
    unsigned n = 0;
    while (1) {
        led = !led;
        gpio_set_level(HEXNET_IO_RUN_LED_GPIO, led ? 1 : 0);
        if ((n++ % 4u) == 0u) {
            ESP_LOGE(TAG, "toggle gpio%d=%d", (int)HEXNET_IO_RUN_LED_GPIO, led ? 1 : 0);
        }
        vTaskDelay(pdMS_TO_TICKS(LED_MS));
    }
#else
    ESP_LOGE(TAG, "STEP1 relay walk: %d channels SR clk/data/stb=%d/%d/%d LED=%d",
             HEXNET_IO_RELAY_COUNT_DEFAULT,
             (int)HEXNET_IO_SR_CLK_GPIO,
             (int)HEXNET_IO_SR_DATA_GPIO,
             (int)HEXNET_IO_SR_STB_GPIO,
             (int)HEXNET_IO_RUN_LED_GPIO);

    int relay = 0;
    while (1) {
#if HEXNET_HAS_RELAY
        uint16_t mask = (relay < HEXNET_IO_RELAY_COUNT_DEFAULT) ? (uint16_t)(1u << relay) : 0;
        sr_write(mask);
        ESP_LOGE(TAG, "relay %d/%d ON mask=0x%04x",
                 relay + 1, HEXNET_IO_RELAY_COUNT_DEFAULT, (unsigned)mask);
        gpio_set_level(HEXNET_IO_RUN_LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(RELAY_HOLD_MS));

        sr_write(0);
        gpio_set_level(HEXNET_IO_RUN_LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(RELAY_GAP_MS));

        relay++;
        if (relay >= HEXNET_IO_RELAY_COUNT_DEFAULT) {
            relay = 0;
            ESP_LOGE(TAG, "relay walk cycle done — repeat");
            vTaskDelay(pdMS_TO_TICKS(800));
        }
#else
        vTaskDelay(pdMS_TO_TICKS(LED_MS));
#endif
    }
#endif
}

#else /* HEXNET_IS_FULL */

void app_main(void)
{
    hexnet_board_gpio_init();
    hexnet_board_start_status_led();

    ESP_LOGI(TAG, "FULL startup | heap=%" PRIu32 " | IDF %s",
             esp_get_free_heap_size(), esp_get_idf_version());

    ota_manager_init();

    esp_err_t nvs_ret = nvs_flash_init();
    if (nvs_ret == ESP_ERR_NVS_NO_FREE_PAGES || nvs_ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvs_ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(nvs_ret);

    hexnet_log_init();
    hexnet_cjson_use_psram();
    hexnet_io_profile_init();
    /* Röle SR hemen — WiFi/MQTT/CAN gecikmesini beklemesin */
    xTaskCreate(hexnet_shift_register_task, "shift_reg", 2048, NULL, 6, NULL);
    hexnet_mqtt_init();
    hexnet_wifi_init();
    /* BLE bring-up sonrasinda (MQTT online once) — heap icin */
    xTaskCreate(hexnet_app_bringup_task, "app_bringup", 4096, NULL, 4, NULL);
}

#endif
