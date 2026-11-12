#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void hexnet_shift_register_task(void *arg);
/** Anında get_outputs() → 74HC4094 (set_outputs sonrası çağrılır). */
void hexnet_shift_register_update_now(void);

#ifdef __cplusplus
}
#endif
