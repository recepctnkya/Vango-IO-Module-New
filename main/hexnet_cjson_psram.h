#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** cJSON alloc → PSRAM (BLE bagliyken internal heap 7KB kaliyor). */
void hexnet_cjson_use_psram(void);

#ifdef __cplusplus
}
#endif
