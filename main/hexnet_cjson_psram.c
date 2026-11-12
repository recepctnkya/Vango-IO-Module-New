#include "hexnet_cjson_psram.h"

#include "cJSON.h"
#include "esp_heap_caps.h"
#include "esp_log.h"

static const char *TAG = "HEXNET_CJSON";

static void *hexnet_cjson_malloc(size_t size)
{
    void *p = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!p) {
        p = heap_caps_malloc(size, MALLOC_CAP_8BIT);
    }
    return p;
}

static void hexnet_cjson_free(void *ptr)
{
    heap_caps_free(ptr);
}

void hexnet_cjson_use_psram(void)
{
    cJSON_Hooks hooks = {
        .malloc_fn = hexnet_cjson_malloc,
        .free_fn = hexnet_cjson_free,
    };
    cJSON_InitHooks(&hooks);
    ESP_LOGI(TAG, "cJSON → PSRAM (free SPIRAM=%u)",
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}
