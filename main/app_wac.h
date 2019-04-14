
#ifndef app_wac_h

#define app_wac_h

#ifdef ESP_TARGET
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <nvs_flash.h>
#endif


    #define func_l  16
    #define p_t_l  6
    #define p_l    12
    #define res_l  12

typedef struct func_call_struct
    {
        char func[func_l];
        char p1_t[p_t_l];
        char p1[p_l];
        char res[res_l];
    } func_call_t;

void parseWasm(unsigned char *bytes, size_t byte_count);

esp_err_t runWasm(func_call_t *funcCall);


#endif