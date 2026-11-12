#include "adc_monitor.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "hexnet_log.h"
#define LOG_LOCAL_LEVEL HEXNET_LOG_LEVEL_OTHER
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hexnet_canbus.h"

#define TAG "ADC_MONITOR"

#define BATTERY_RAW_AT_11V5   1017
#define BATTERY_RAW_AT_14V5   1470
#define WATERLEVEL_BUF_SIZE   16
#define ADC_SAMPLE_COUNT      32

/*
 * Temiz su (GPIO4): klasik — dusuk ADC = bos, yuksek = dolu.
 * Olcu: bos raw~0.
 */
#define CLEAN_RAW_EMPTY   40
#define CLEAN_RAW_FULL    1649

/*
 * Kirli su (GPIO5): raw ARTINCA % artar (onceki falling bos/dolu tersidi).
 * Orta seviye raw~2050 ≈ %50 → bos~0, dolu~4100.
 */
#define DIRTY_RAW_EMPTY   50
#define DIRTY_RAW_FULL    4100

/* Gercekten kopuk (ADC tavan) — opsiyonel ayir; 4100 dolu ile cakismasin */
#define RAW_OPEN_CIRCUIT  4200

static int waterLevel1_buf[WATERLEVEL_BUF_SIZE] = {0};
static int waterLevel2_buf[WATERLEVEL_BUF_SIZE] = {0};
static int waterLevel_buf_idx = 0;
int waterLevel1 = 0;
int waterLevel2 = 0;
int waterLevel3 = 0;
int waterLevel4 = 0;

static esp_adc_cal_characteristics_t adc1_chars;

static int average_buf(const int *buf)
{
    int sum = 0;
    for (int i = 0; i < WATERLEVEL_BUF_SIZE; i++) {
        sum += buf[i];
    }
    return sum / WATERLEVEL_BUF_SIZE;
}

void get_average_water_levels(int *pWaterLevel1, int *pWaterLevel2, int *pWaterLevel3, int *pWaterLevel4)
{
    if (pWaterLevel1) {
        *pWaterLevel1 = average_buf(waterLevel1_buf);
    }
    if (pWaterLevel2) {
        *pWaterLevel2 = average_buf(waterLevel2_buf);
    }
    if (pWaterLevel3) {
        *pWaterLevel3 = 0;
    }
    if (pWaterLevel4) {
        *pWaterLevel4 = 0;
    }
}

int hexnet_get_water_level(int index)
{
    switch (index) {
        case 0: return waterLevel1;
        case 1: return waterLevel2;
        case 2: return waterLevel3;
        case 3: return waterLevel4;
        default: return 0;
    }
}

static void init_adc1_channels(void)
{
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC1_SENSOR_CH0, ADC_ATTEN); /* GPIO3 Analog1 */
    adc1_config_channel_atten(ADC1_SENSOR_CH1, ADC_ATTEN); /* GPIO9 Analog2 */
    adc1_config_channel_atten(ADC2_BATTERY_CHANNEL, ADC_ATTEN); /* GPIO4 Vin */
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN, ADC_WIDTH, DEFAULT_VREF, &adc1_chars);
}

static uint32_t read_adc1_avg(adc1_channel_t channel)
{
    uint32_t sum = 0;
    for (int i = 0; i < ADC_SAMPLE_COUNT; i++) {
        sum += (uint32_t)adc1_get_raw(channel);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return sum / ADC_SAMPLE_COUNT;
}

static float calculate_input_voltage(uint32_t raw)
{
    return 10.0f + ((float)(raw - BATTERY_RAW_AT_11V5) / (BATTERY_RAW_AT_14V5 - BATTERY_RAW_AT_11V5)) * (14.14f - 10.0f);
}

static int clamp_pct(int pct)
{
    if (pct < 0) {
        return 0;
    }
    if (pct > 100) {
        return 100;
    }
    return pct;
}

/** Temiz: raw artinca % artar */
static int pct_clean_rising(int raw)
{
    if (raw >= RAW_OPEN_CIRCUIT) {
        return 0;
    }
    if (raw <= CLEAN_RAW_EMPTY) {
        return 0;
    }
    if (raw >= CLEAN_RAW_FULL) {
        return 100;
    }
    return clamp_pct(((raw - CLEAN_RAW_EMPTY) * 100) / (CLEAN_RAW_FULL - CLEAN_RAW_EMPTY));
}

/** Kirli: raw artinca % artar (bos~50, dolu~4100; orta~2050≈%50) */
static int pct_dirty_rising(int raw)
{
    if (raw <= DIRTY_RAW_EMPTY) {
        return 0;
    }
    if (raw >= DIRTY_RAW_FULL) {
        return 100;
    }
    return clamp_pct(((raw - DIRTY_RAW_EMPTY) * 100) / (DIRTY_RAW_FULL - DIRTY_RAW_EMPTY));
}

void adc_monitor_init(void)
{
    init_adc1_channels();
}

void adc_monitor_task(void *param)
{
    (void)param;
    while (1) {
        int battery_raw = (int)read_adc1_avg(ADC2_BATTERY_CHANNEL);
        float voltage = calculate_input_voltage((uint32_t)battery_raw);
        get_voltage((uint16_t)(voltage * 100));

        int raw0 = (int)read_adc1_avg(ADC1_SENSOR_CH0);
        int raw1 = (int)read_adc1_avg(ADC1_SENSOR_CH1);

        int w1 = pct_clean_rising(raw0);
        int w2 = pct_dirty_rising(raw1);

        waterLevel1_buf[waterLevel_buf_idx] = w1;
        waterLevel2_buf[waterLevel_buf_idx] = w2;
        waterLevel_buf_idx = (waterLevel_buf_idx + 1) % WATERLEVEL_BUF_SIZE;

        waterLevel1 = average_buf(waterLevel1_buf);
        waterLevel2 = average_buf(waterLevel2_buf);
        waterLevel3 = 0;
        waterLevel4 = 0;

        static int s_log_div = 0;
        if ((++s_log_div % 20) == 0) {
            ESP_LOGE(TAG, "ADC temiz raw=%d -> %d%% | kirli raw=%d -> %d%% | bat=%.2fV",
                     raw0, waterLevel1, raw1, waterLevel2, voltage);
        }

        update_water_levels_only(waterLevel1, waterLevel2);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
