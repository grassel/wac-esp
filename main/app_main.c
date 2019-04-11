#ifdef ESP_TARGET
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <nvs_flash.h>
#endif

#include "app_wac.h"
#include "app_wifi_http.h"


int main(int argc, char **argv)
{
    //InitializeWac();

#ifdef ESP_TARGET
    //printf("Restarting now.\n");
    //fflush(stdout);
    //esp_restart();
    return 0;
#else
    return 0;
#endif
}
/**
 * ESP32 specific intialisation
 * This function must be called before anything else
 */
void initializeEsp()
{
    // non-volatile storrage library (seems needed by Wifi)
    ESP_ERROR_CHECK(nvs_flash_init());
}

// entrypoint for ESP
void app_main()
{
    initializeEsp();
    initialiseWifiAndStartServer();
}