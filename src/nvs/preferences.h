#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "nvs_flash.h"
#include "nvs.h"


#ifdef __cplusplus
extern "C" {
#endif

char* get_string_from_nvs(nvs_handle_t handle, const char* key, const char* default_value);
size_t put_string_to_nvs(nvs_handle_t handle, const char* key, const char* value);
uint8_t get_uint8_from_nvs(nvs_handle_t handle, const char* key, uint8_t default_value);
size_t put_uint8_to_nvs(nvs_handle_t handle, const char* key, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif /* PREFERENCES_H */