#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef ESP_TARGET
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <esp_heap_caps.h>
#endif

#include "util.h"

// Assert calloc
void *acalloc(size_t nmemb, size_t size, char *name)
{
    void *res = calloc(nmemb, size);
    if (nmemb * size == 0)
    {
        warn("acalloc: %s requests allocating 0 bytes.\n", name);
    }
    else if (res == NULL)
    {
        FATAL("Could not allocate %ul bytes for %s", nmemb * size, name);
    }
    return res;
}

// Assert realloc/calloc
void *arecalloc(void *ptr, size_t old_nmemb, size_t nmemb,
                size_t size, char *name)
{
    void *res = realloc(ptr, nmemb * size);
    if (res == NULL)
    {
        FATAL("Could not allocate %ul bytes for %s", nmemb * size, name);
    }
    // Initialize new memory
    memset(res + old_nmemb * size, 0, (nmemb - old_nmemb) * size);
    return res;
}

/**
 *  dumpMemoryInfo - RTOS specific function to trace available heap memory
 *
 */
void dumpMemoryInfo()
{
#ifdef ESP_TARGET
#if TRACE
    heap_caps_print_heap_info(MALLOC_CAP_8BIT);
#endif
#endif
}