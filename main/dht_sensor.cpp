#include "dht_sensor.h"
#include "TroykaDHT.h"
#include "esp_log.h"
#include <map>

static const char *TAG = "DHT_DRV";
static std::map<int, DHT*> dht_sensors;

extern "C" void dht_init(int gpio_pin) {
    if (dht_sensors.find(gpio_pin) == dht_sensors.end()) {
        DHT* sensor = new DHT(static_cast<gpio_num_t>(gpio_pin), DHT22);
        sensor->begin();
        dht_sensors[gpio_pin] = sensor;
    }
}

extern "C" int dht_read_data(float *temperature, float *humidity) {
    if (dht_sensors.empty()) return -1;

    DHT* sensor = dht_sensors.begin()->second;
    int8_t result = sensor->read();
    if (result == DHT_OK) {
        *temperature = sensor->getTemperatureC();
        *humidity = sensor->getHumidity();
        return 0;
    }
    ESP_LOGE(TAG, "DHT read rc=%d", (int)result);
    return (int)result;
}

extern "C" int dht_read_data_gpio(int gpio_pin, float *temperature, float *humidity) {
    auto it = dht_sensors.find(gpio_pin);
    if (it == dht_sensors.end()) {
        dht_init(gpio_pin);
        it = dht_sensors.find(gpio_pin);
        if (it == dht_sensors.end()) return -1;
    }

    DHT* sensor = it->second;
    int8_t result = sensor->read();
    if (result == DHT_OK) {
        *temperature = sensor->getTemperatureC();
        *humidity = sensor->getHumidity();
        return 0;
    }
    ESP_LOGE(TAG, "GPIO%d DHT rc=%d", gpio_pin, (int)result);
    return (int)result;
}
