
#ifndef app_wac_h

#define app_wac_h

#ifdef ESP_TARGET
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <nvs_flash.h>
#endif


void parseWasm(unsigned char *bytes, size_t byte_count);

void runWasm();


#endif