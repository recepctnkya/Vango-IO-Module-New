#include "hexnet_io_notify.h"

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "hexnet_bluetooth.h"
#include "hexnet_canbus.h"
#include "hexnet_mqtt.h"

static const char *TAG = "HEXNET_IO_SYNC";

void hexnet_io_notify_state_changed(void)
{
    /* BLE RGB mirror = HW (MQTT rgb_set sonrasi tablet eski kalmasin) */
    hexnet_ble_mirror_rgb_from_hw();

    /*
     * Once BLE (lean, heap-safe), sonra MQTT.
     * BLE bagliyken ek MQTT cJSON (PSRAM kapali) OOM/reset riski → esik altinda atla;
     * periyodik telemetry zaten ~3s'de gider.
     */
    hexnet_ble_push_telemetry_now();
    const size_t iheap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    if (iheap >= 28000u) {
        hexnet_mqtt_publish_telemetry_now();
    } else {
        ESP_LOGW(TAG, "mqtt force-publish skipped (iheap=%u)", (unsigned)iheap);
    }
    ESP_LOGI(TAG, "state pushed ble+mqtt mask=0x%04X", (unsigned)get_outputs());
}
