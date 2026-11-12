#ifndef ADC_MONITOR_H
#define ADC_MONITOR_H

#include "esp_adc_cal.h"

#define ADC_WIDTH       ADC_WIDTH_BIT_12
#define ADC_ATTEN       ADC_ATTEN_DB_11
#define DEFAULT_VREF    1100 // in mV

/* v0.60 pin map: GPIO3/4/9/10 */
#define ADC2_BATTERY_CHANNEL  ADC1_CHANNEL_3   // GPIO4 — Input_Voltage_MCU
#define ADC1_SENSOR_CH0       ADC1_CHANNEL_2   // GPIO3 — AnalogInputMCU1 (clean)
#define ADC1_SENSOR_CH1       ADC1_CHANNEL_8   // GPIO9 — AnalogInputMCU2 (dirty)
#define ADC1_SENSOR_CH2       ADC1_CHANNEL_9   // GPIO10 — AnalogInputMCU3

void adc_monitor_init(void);
void adc_monitor_task(void *param);

/** 0..100 yuzde; gecersiz/kopuk sensor icin 0. */
int hexnet_get_water_level(int index);

#endif // ADC_MONITOR_H
