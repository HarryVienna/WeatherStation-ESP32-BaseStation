#include "esp_log.h"
#include <string.h>

#include "preferences.h"

static const char* TAG = "PREFS";

// Placeholder function for converting esp_err_t to string
const char* nvs_error(esp_err_t err) {
    return esp_err_to_name(err);
}

// Function to get a string from NVS
char* get_string_from_nvs(nvs_handle_t handle, const char* key, const char* default_value) {

    size_t len = 0;
    esp_err_t err = nvs_get_str(handle, key, NULL, &len);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(TAG, "nvs_get_str len fail: %s %s", key, nvs_error(err));
        return strdup(default_value);
    }

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        return strdup(default_value);
    }

    char* value = (char*)malloc(len);
    if (value == NULL) {
        ESP_LOGE(TAG, "Memory allocation failed for key: %s", key);
        return strdup(default_value);
    }

    err = nvs_get_str(handle, key, value, &len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_get_str fail: %s %s", key, nvs_error(err));
        free(value);
        return strdup(default_value);
    }

    return value;
}

// Function to put a string into NVS
size_t put_string_to_nvs(nvs_handle_t handle, const char* key, const char* value) {

    esp_err_t err = nvs_set_str(handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_str fail: %s %s", key, nvs_error(err));
        return 0;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }

    return strlen(value);
}

// Function to get an unsigned integer from NVS
uint8_t get_uint8_from_nvs(nvs_handle_t handle, const char* key, uint8_t default_value) {
    uint8_t value = default_value;

    esp_err_t err = nvs_get_u8(handle, key, &value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_get_u32 fail: %s %s", key, nvs_error(err));
    }

    return value;
}

// Function to put an unsigned integer into NVS
size_t put_uint8_to_nvs(nvs_handle_t handle, const char* key, uint8_t value) {

    esp_err_t err = nvs_set_u8(handle, key, value);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"nvs_set_u32 fail: %s %s", key, nvs_error(err));
        return 0;
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG,"nvs_commit fail: %s %s", key, nvs_error(err));
        return 0;
    }

    return sizeof(uint8_t);
}