/**
 * @file hexnet_io_map.h
 * @brief Hexnet IO Module v0.60 – ESP32-S3-WROOM-1U-N8R8 pin map
 *
 * Donanim: 12 role (shift register) + 8 dimmer (MOSFET PWM)
 *
 *  ┌────────┬──────────────────────────────────────────────────────────────┐
 *  │  GPIO  │  Function                                                    │
 *  ├────────┼──────────────────────────────────────────────────────────────┤
 *  │   1    │  SensorDetect                                                │
 *  │   2    │  WS2812 RGB data                                             │
 *  │   3    │  ADC – AnalogInputMCU1                                       │
 *  │   4    │  ADC – Input_Voltage_MCU                                     │
 *  │   5    │  Dimmer PWM #1 (Mosfet-MCU-1)                                │
 *  │   6    │  Dimmer PWM #2 (Mosfet-MCU-2)                                │
 *  │   7    │  Dimmer PWM #3 (Mosfet-MCU-3)                                │
 *  │   8    │  Dimmer PWM #8 (Mosfet-MCU-8)                                │
 *  │   9    │  ADC – AnalogInputMCU2                                       │
 *  │  10    │  ADC – AnalogInputMCU3                                       │
 *  │  11    │  Motor1 IN A                                                 │
 *  │  12    │  Motor1 IN B                                                 │
 *  │  13    │  Motor2 IN A                                                 │
 *  │  14    │  Motor2 IN B                                                 │
 *  │  15    │  Dimmer PWM #4 (Mosfet-MCU-4)                                │
 *  │  16    │  Dimmer PWM #5 (Mosfet-MCU-5)                                │
 *  │  17    │  Dimmer PWM #6 (Mosfet-MCU-6)                                │
 *  │  18    │  Dimmer PWM #7 (Mosfet-MCU-7)                                │
 *  │  19    │  Status LED (green)                                          │
 *  │  20    │  (NC / reserved)                                             │
 *  │  21    │  Motor PWM / enable                                          │
 *  │  35*   │  ShiftReg STB  (*N8R8 Octal PSRAM ile cakisma riski)         │
 *  │  36*   │  ShiftReg DATA (*N8R8 Octal PSRAM ile cakisma riski)         │
 *  │  37*   │  ShiftReg CLK  (*N8R8 Octal PSRAM ile cakisma riski)         │
 *  │  38    │  UART1 TX – RS485                                            │
 *  │  39    │  UART1 RX – RS485                                            │
 *  │  40    │  RS485 DE/RE                                                 │
 *  │  41    │  Debug UART TX (FT232; bootloader UART0 degil)               │
 *  │  42    │  Debug UART RX (FT232; bootloader UART0 degil)               │
 *  │  43    │  U0TXD – ROM flash (FT232 buraya baglanmali)                 │
 *  │  44    │  U0RXD – ROM flash                                           │
 *  │  45    │  (strap / reserved)                                          │
 *  │  46    │  (strap / reserved)                                          │
 *  │  47    │  CAN TX                                                      │
 *  │  48    │  CAN RX                                                      │
 *  └────────┴──────────────────────────────────────────────────────────────┘
 *
 * Role maskesi: bit0..bit11 → 12 role (shift register zinciri, 16-bit shift).
 * Dimmer: 8x LEDC → Mosfet-MCU-1..8.
 */
#pragma once

#include "driver/gpio.h"
#include "driver/uart.h"

/* --- Kapasite --- */
#define HEXNET_IO_RELAY_COUNT_DEFAULT   12
#define HEXNET_IO_DIM_COUNT_DEFAULT     8

/* --- Status LED --- */
#define HEXNET_IO_RUN_LED_GPIO          GPIO_NUM_19

/* --- Shift register (12 role / 16-bit zincir) ---
 * NOT: ESP32-S3R8 Octal PSRAM IO35/36/37 kullanir. Sema bu pinleri SR icin
 * ayirmis; PCB N8R8 ise PSRAM ile cakisabilir — donanim dogrulanmali.
 */
#define HEXNET_IO_SR_CLK_GPIO           GPIO_NUM_37
#define HEXNET_IO_SR_DATA_GPIO          GPIO_NUM_36
#define HEXNET_IO_SR_STB_GPIO           GPIO_NUM_35

/* --- Motor (M1 birincil; M2 pinleri hazir) --- */
#define HEXNET_IO_MOTOR_EN_GPIO         GPIO_NUM_21
#define HEXNET_IO_MOTOR_A_GPIO          GPIO_NUM_11
#define HEXNET_IO_MOTOR_B_GPIO          GPIO_NUM_12
#define HEXNET_IO_MOTOR2_A_GPIO         GPIO_NUM_13
#define HEXNET_IO_MOTOR2_B_GPIO         GPIO_NUM_14

/* --- Dimmer PWM (LEDC) — Mosfet-MCU-1..8 --- */
#define HEXNET_IO_DIM_PWM_GPIO_1        GPIO_NUM_5
#define HEXNET_IO_DIM_PWM_GPIO_2        GPIO_NUM_6
#define HEXNET_IO_DIM_PWM_GPIO_3        GPIO_NUM_7
#define HEXNET_IO_DIM_PWM_GPIO_4        GPIO_NUM_15
#define HEXNET_IO_DIM_PWM_GPIO_5        GPIO_NUM_16
#define HEXNET_IO_DIM_PWM_GPIO_6        GPIO_NUM_17
#define HEXNET_IO_DIM_PWM_GPIO_7        GPIO_NUM_18
#define HEXNET_IO_DIM_PWM_GPIO_8        GPIO_NUM_8

/* --- WS2812 RGB --- */
#define HEXNET_IO_RGB_STRIP_GPIO        GPIO_NUM_2
#define HEXNET_IO_RGB_LED_COUNT         65
#define HEXNET_IO_RGB_MATRIX_ROWS       10
#define HEXNET_IO_RGB_MATRIX_COLS       20
#define HEXNET_IO_RGB_MATRIX_LEDS       (HEXNET_IO_RGB_MATRIX_ROWS * HEXNET_IO_RGB_MATRIX_COLS)

/* --- Sensor detect --- */
#define HEXNET_IO_SENSOR_DETECT_GPIO    GPIO_NUM_1

/* --- ADC (yazilim kanal eslemesi adc_monitor icinde) --- */
#define HEXNET_IO_ADC_ANALOG1_GPIO      GPIO_NUM_3
#define HEXNET_IO_ADC_ANALOG2_GPIO      GPIO_NUM_9
#define HEXNET_IO_ADC_ANALOG3_GPIO      GPIO_NUM_10
#define HEXNET_IO_ADC_VIN_GPIO          GPIO_NUM_4

/* --- Eski MUX yok (motor pinleri ile cakisirdi); board init no-op mask --- */
#define HEXNET_IO_MUX_S0_GPIO           GPIO_NUM_NC
#define HEXNET_IO_MUX_S1_GPIO           GPIO_NUM_NC
#define HEXNET_IO_MUX_S2_GPIO           GPIO_NUM_NC
#define HEXNET_IO_MUX_Z_A_GPIO          GPIO_NUM_NC
#define HEXNET_IO_MUX_Z_B_GPIO          GPIO_NUM_NC
#define HEXNET_IO_MUX_CHANNEL_COUNT     0

/* --- DHT yedek (SensorDetect konnektoru). Iklim birincil SoT: RS485. --- */
#define HEXNET_IO_DHT_GPIO_PRIMARY      GPIO_NUM_1
#define HEXNET_IO_DHT_GPIO_ALT          GPIO_NUM_NC

/* --- RS485 / UART1 --- */
#define HEXNET_IO_RS485_EN_GPIO         GPIO_NUM_40
#define HEXNET_IO_UART_RS485_PORT       UART_NUM_1
#define HEXNET_IO_UART_RS485_TX_GPIO    GPIO_NUM_38
#define HEXNET_IO_UART_RS485_RX_GPIO    GPIO_NUM_39
#define HEXNET_IO_UART_RS485_BAUD       115200
#define HEXNET_IO_UART_RS485_BUF_SIZE   512

/* --- CAN / TWAI --- */
#define HEXNET_IO_CAN_TX_GPIO           GPIO_NUM_47
#define HEXNET_IO_CAN_RX_GPIO           GPIO_NUM_48

/* --- Debug UART (FT232) — uygulama log; flash icin U0TXD/U0RXD (43/44) kullanin --- */
#define HEXNET_IO_DEBUG_UART_TX_GPIO    GPIO_NUM_41
#define HEXNET_IO_DEBUG_UART_RX_GPIO    GPIO_NUM_42

/* Bit masks for gpio_config */
#define HEXNET_IO_MUX_SEL_MASK          (0ULL)
#define HEXNET_IO_MUX_Z_MASK            (0ULL)
#define HEXNET_IO_SR_MASK \
    ((1ULL << HEXNET_IO_SR_CLK_GPIO) | (1ULL << HEXNET_IO_SR_DATA_GPIO) | (1ULL << HEXNET_IO_SR_STB_GPIO))
#define HEXNET_IO_MOTOR_MASK \
    ((1ULL << HEXNET_IO_MOTOR_EN_GPIO) | (1ULL << HEXNET_IO_MOTOR_A_GPIO) | (1ULL << HEXNET_IO_MOTOR_B_GPIO) | \
     (1ULL << HEXNET_IO_MOTOR2_A_GPIO) | (1ULL << HEXNET_IO_MOTOR2_B_GPIO))
#define HEXNET_IO_DIM_PWM_MASK \
    ((1ULL << HEXNET_IO_DIM_PWM_GPIO_1) | (1ULL << HEXNET_IO_DIM_PWM_GPIO_2) | \
     (1ULL << HEXNET_IO_DIM_PWM_GPIO_3) | (1ULL << HEXNET_IO_DIM_PWM_GPIO_4) | \
     (1ULL << HEXNET_IO_DIM_PWM_GPIO_5) | (1ULL << HEXNET_IO_DIM_PWM_GPIO_6) | \
     (1ULL << HEXNET_IO_DIM_PWM_GPIO_7) | (1ULL << HEXNET_IO_DIM_PWM_GPIO_8))
