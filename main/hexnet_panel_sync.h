#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Panel SoT degisince: CAN + MQTT + BLE push (hibrit sync). */
void hexnet_panel_sync_on_changed(void);

/** CAN heartbeat / dirty: true ise 0x110 gonderilmeli. */
bool hexnet_panel_sync_can_should_send(bool heartbeat_due);

/** CAN 0x110 basariyla gonderildi. */
void hexnet_panel_sync_can_sent(void);

/** panel_seq (NVS kalici, her profile save'de artar). */
uint16_t hexnet_panel_sync_get_seq(void);

/**
 * true = bu publish degisiklik aninda (Platform/BLE: DB/cache guncelle).
 * Heartbeat/periyodik telemetride false.
 */
bool hexnet_panel_sync_is_changed_publish(void);

#ifdef __cplusplus
}
#endif
