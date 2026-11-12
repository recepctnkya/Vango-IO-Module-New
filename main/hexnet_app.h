#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** Deferred init: CAN, IO tasks, MQTT (after Wi-Fi portal window). */
void hexnet_app_bringup_task(void *arg);

/** Start BLE advertising ASAP (independent of Wi-Fi/MQTT). Safe to call once. */
void hexnet_app_start_ble(void);

#ifdef __cplusplus
}
#endif
