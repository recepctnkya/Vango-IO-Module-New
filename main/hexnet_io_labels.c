#include "hexnet_io_labels.h"

#include <string.h>

#include "esp_log.h"
#include "hexnet_io_profile.h"
#include "nvs.h"

static const char *TAG = "IO_LABELS";

#define NVS_NS   "hexnet"
#define NVS_KEY  "io_labels"
#define MAGIC    0x4C4F4948u /* HIOL */

typedef struct __attribute__((packed)) {
    uint32_t magic;
    uint8_t version;
    uint8_t reserved[3];
    hexnet_io_labels_t labels;
} hexnet_io_labels_blob_t;

static hexnet_io_labels_t s_labels;
static bool s_loaded;

static void slot_clear(hexnet_label_slot_t *s)
{
    if (!s) {
        return;
    }
    s->name[0] = '\0';
    s->icon[0] = '\0';
}

static void labels_clear(hexnet_io_labels_t *L)
{
    for (int i = 0; i < HEXNET_LABEL_RELAYS; ++i) {
        slot_clear(&L->relays[i]);
    }
    for (int i = 0; i < HEXNET_LABEL_DIMS; ++i) {
        slot_clear(&L->dimmers[i]);
    }
    for (int i = 0; i < HEXNET_LABEL_SENSORS; ++i) {
        slot_clear(&L->sensors[i]);
    }
    for (int i = 0; i < HEXNET_LABEL_WATER; ++i) {
        slot_clear(&L->water[i]);
    }
}

static void copy_cstr(char *dst, size_t dst_sz, const char *src)
{
    if (!dst || dst_sz == 0) {
        return;
    }
    if (!src) {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, dst_sz - 1);
    dst[dst_sz - 1] = '\0';
}

static void apply_slot_obj(hexnet_label_slot_t *slot, const cJSON *item)
{
    if (!slot || !cJSON_IsObject(item)) {
        return;
    }
    const cJSON *name = cJSON_GetObjectItem(item, "name");
    const cJSON *icon = cJSON_GetObjectItem(item, "icon");
    if (cJSON_IsString(name) && name->valuestring) {
        copy_cstr(slot->name, sizeof(slot->name), name->valuestring);
    }
    if (cJSON_IsString(icon) && icon->valuestring) {
        copy_cstr(slot->icon, sizeof(slot->icon), icon->valuestring);
    }
}

static void apply_channel_array(hexnet_label_slot_t *slots, int max_slots, const cJSON *arr)
{
    if (!cJSON_IsArray(arr) || !slots) {
        return;
    }
    const int n = cJSON_GetArraySize(arr);
    for (int i = 0; i < n; ++i) {
        const cJSON *item = cJSON_GetArrayItem(arr, i);
        if (!cJSON_IsObject(item)) {
            continue;
        }
        const cJSON *id_item = cJSON_GetObjectItem(item, "id");
        int id = cJSON_IsNumber(id_item) ? id_item->valueint : i;
        if (id < 0 || id >= max_slots) {
            continue;
        }
        apply_slot_obj(&slots[id], item);
    }
}

static cJSON *slot_to_json(int id, const hexnet_label_slot_t *slot, int enabled)
{
    cJSON *o = cJSON_CreateObject();
    if (!o) {
        return NULL;
    }
    cJSON_AddNumberToObject(o, "id", id);
    cJSON_AddBoolToObject(o, "enabled", enabled != 0);
    cJSON_AddStringToObject(o, "name", slot && slot->name[0] ? slot->name : "");
    cJSON_AddStringToObject(o, "icon", slot && slot->icon[0] ? slot->icon : "");
    return o;
}

void hexnet_io_labels_init(void)
{
    labels_clear(&s_labels);
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READONLY, &h) != ESP_OK) {
        s_loaded = true;
        ESP_LOGW(TAG, "labels NVS yok — bos");
        return;
    }
    hexnet_io_labels_blob_t blob;
    size_t len = sizeof(blob);
    esp_err_t err = nvs_get_blob(h, NVS_KEY, &blob, &len);
    nvs_close(h);
    if (err == ESP_OK && len >= sizeof(blob) && blob.magic == MAGIC && blob.version == 1) {
        s_labels = blob.labels;
        ESP_LOGI(TAG, "labels NVS yuklendi");
    } else {
        ESP_LOGW(TAG, "labels blob yok/bozuk");
    }
    s_loaded = true;
}

const hexnet_io_labels_t *hexnet_io_labels_get(void)
{
    return &s_labels;
}

esp_err_t hexnet_io_labels_save(void)
{
    hexnet_io_labels_blob_t blob;
    memset(&blob, 0, sizeof(blob));
    blob.magic = MAGIC;
    blob.version = 1;
    blob.labels = s_labels;

    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NS, NVS_READWRITE, &h);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_set_blob(h, NVS_KEY, &blob, sizeof(blob));
    if (err == ESP_OK) {
        err = nvs_commit(h);
    }
    nvs_close(h);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "labels flash'a yazildi (%u B)", (unsigned)sizeof(blob));
    }
    return err;
}

bool hexnet_io_labels_apply_json(const cJSON *json)
{
    if (!json || !s_loaded) {
        return false;
    }
    const cJSON *channels = cJSON_GetObjectItem(json, "channels");
    if (!cJSON_IsObject(channels)) {
        channels = cJSON_GetObjectItem(json, "panel_channels");
    }
    if (!cJSON_IsObject(channels)) {
        return false;
    }

    bool any = false;
    const cJSON *relays = cJSON_GetObjectItem(channels, "relays");
    if (cJSON_IsArray(relays)) {
        apply_channel_array(s_labels.relays, HEXNET_LABEL_RELAYS, relays);
        any = true;
    }
    const cJSON *dims = cJSON_GetObjectItem(channels, "dimmers");
    if (cJSON_IsArray(dims)) {
        apply_channel_array(s_labels.dimmers, HEXNET_LABEL_DIMS, dims);
        any = true;
    }
    const cJSON *sensors = cJSON_GetObjectItem(channels, "sensors");
    if (cJSON_IsArray(sensors)) {
        apply_channel_array(s_labels.sensors, HEXNET_LABEL_SENSORS, sensors);
        any = true;
    }
    const cJSON *water = cJSON_GetObjectItem(channels, "water");
    if (cJSON_IsArray(water)) {
        apply_channel_array(s_labels.water, HEXNET_LABEL_WATER, water);
        any = true;
    }

    if (any) {
        (void)hexnet_io_labels_save();
    }
    return any;
}

void hexnet_io_labels_append_channels(cJSON *io_obj)
{
    if (!io_obj) {
        return;
    }
    const hexnet_io_profile_t *prof = hexnet_io_profile_get();

    cJSON *channels = cJSON_CreateObject();
    cJSON *relays = cJSON_CreateArray();
    cJSON *dims = cJSON_CreateArray();
    cJSON *sensors = cJSON_CreateArray();
    cJSON *water = cJSON_CreateArray();
    if (!channels || !relays || !dims || !sensors || !water) {
        cJSON_Delete(channels);
        cJSON_Delete(relays);
        cJSON_Delete(dims);
        cJSON_Delete(sensors);
        cJSON_Delete(water);
        return;
    }

    const int n_rel = (prof && prof->num_relays > 0) ? prof->num_relays : 12;
    for (int i = 0; i < n_rel && i < HEXNET_LABEL_RELAYS; ++i) {
        cJSON_AddItemToArray(
            relays,
            slot_to_json(i, &s_labels.relays[i], hexnet_io_profile_relay_slot_enabled((uint8_t)i) ? 1 : 0));
    }
    const int n_dim = (prof && prof->num_dims > 0) ? prof->num_dims : 8;
    for (int i = 0; i < n_dim && i < HEXNET_LABEL_DIMS; ++i) {
        cJSON_AddItemToArray(
            dims,
            slot_to_json(i, &s_labels.dimmers[i], hexnet_io_profile_dim_slot_enabled((uint8_t)i) ? 1 : 0));
    }
    for (int i = 0; i < HEXNET_LABEL_SENSORS; ++i) {
        cJSON_AddItemToArray(
            sensors,
            slot_to_json(i, &s_labels.sensors[i], hexnet_io_profile_sensor_slot_enabled((uint8_t)i) ? 1 : 0));
    }
    for (int i = 0; i < HEXNET_LABEL_WATER; ++i) {
        const int en = (prof && (prof->tank_enable_mask & (1u << i))) ? 1 : 0;
        cJSON_AddItemToArray(water, slot_to_json(i, &s_labels.water[i], en));
    }

    cJSON_AddItemToObject(channels, "relays", relays);
    cJSON_AddItemToObject(channels, "dimmers", dims);
    cJSON_AddItemToObject(channels, "sensors", sensors);
    cJSON_AddItemToObject(channels, "water", water);
    cJSON_AddItemToObject(io_obj, "channels", channels);
}
