/**
 * Staged bring-up (set HEXNET_BUILD_STEP in main/CMakeLists.txt).
 *
 *  0  Run LED blink only
 *  1  Relays (shift register) + Run LED
 *  2  + Dimmer PWM
 *  3  + RGB strip
 *  4  + water level (ADC tanks)
 *  5  + temperature / humidity
 *  6  + motor
 *  7  + WiFi (+ portal / resolve / MQTT as needed)
 *  8  + BLE
 *  9  + CAN / remaining
 * 99  Full production firmware
 */
#pragma once

#ifndef HEXNET_BUILD_STEP
#define HEXNET_BUILD_STEP 99
#endif

#define HEXNET_STEP_LED     0
#define HEXNET_STEP_RELAY   1
#define HEXNET_STEP_DIMMER  2
#define HEXNET_STEP_RGB     3
#define HEXNET_STEP_LEVEL   4
#define HEXNET_STEP_DHT     5
#define HEXNET_STEP_MOTOR   6
#define HEXNET_STEP_WIFI    7
#define HEXNET_STEP_BLE     8
#define HEXNET_STEP_CAN     9
#define HEXNET_STEP_FULL    99

#define HEXNET_HAS_RELAY   (HEXNET_BUILD_STEP >= HEXNET_STEP_RELAY)
#define HEXNET_HAS_DIMMER  (HEXNET_BUILD_STEP >= HEXNET_STEP_DIMMER)
#define HEXNET_HAS_RGB     (HEXNET_BUILD_STEP >= HEXNET_STEP_RGB)
#define HEXNET_HAS_LEVEL   (HEXNET_BUILD_STEP >= HEXNET_STEP_LEVEL)
#define HEXNET_HAS_DHT     (HEXNET_BUILD_STEP >= HEXNET_STEP_DHT)
#define HEXNET_HAS_MOTOR   (HEXNET_BUILD_STEP >= HEXNET_STEP_MOTOR)
#define HEXNET_HAS_WIFI    (HEXNET_BUILD_STEP >= HEXNET_STEP_WIFI)
#define HEXNET_HAS_BLE     (HEXNET_BUILD_STEP >= HEXNET_STEP_BLE)
#define HEXNET_HAS_CAN     (HEXNET_BUILD_STEP >= HEXNET_STEP_CAN)
#define HEXNET_IS_FULL     (HEXNET_BUILD_STEP >= HEXNET_STEP_FULL)
