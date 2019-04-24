
#include "app_wac.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <esp_log.h>
#include <string.h>

// Call table/trapping table lookups/execution
#include <unistd.h>
#include <signal.h>

#include "util.h"
#include "platform.h"
#include "wa.h"
#include "thunk.h"

#include "app_wifi_http.h"

const char *TAG = "app_wac: ";

/////////////////////////////////////////////////////////
// memory layout

#ifdef LOW_MEMORY_CONFIG
#define PAGE_COUNT 1 // 64K each
#define TOTAL_PAGES 1
#define TABLE_COUNT 2
#else
#define PAGE_COUNT 1                     // 64K each
#define TOTAL_PAGES 0x100000 / PAGE_SIZE // use 1MByte of memory
#define TABLE_COUNT 20
#endif


esp_err_t runWasm(func_call_t *fc)
{
    ESP_LOGI(TAG, "runWasm ..............");

    Module *m = getModule();

    if (m == NULL)
    {
        ESP_LOGW(TAG, "runWasm - no module loaded");
        return ESP_FAIL;
    }

    int fidx = get_export_fidx(m, fc->func);
    if (fidx < 0)
    {
        ESP_LOGW(TAG, "runWasm: WASM module does not have exported function %s", fc->func);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "runWasm: found function %s", fc->func);

    if ((strcmp(fc->p1_t, "I32") == 0) ||
        (strcmp(fc->p1_t, "i32") == 0))
    {
        m->stack[++m->sp].value_type = I32;
        const int p1 = atoi(fc->p1);
        m->stack[m->sp].value.int32 = p1;
        ESP_LOGI(TAG, "  int parameter %i.", p1);
    }
    else if ((strcmp(fc->p1_t, "F32") == 0) ||
             (strcmp(fc->p1_t, "f32") == 0))
    {
        m->stack[++m->sp].value_type = F32;
        const float p1 = atof(fc->p1);
        m->stack[m->sp].value.f32 = p1;
        ESP_LOGI(TAG, "  float parameter %f.", p1);
    }
    // FIXMEL string parameters need to be mem copied
    // FIXME: very correctness of parameer signature and number
    int res = invoke(m, fidx);

    if (!res)
    {
        ESP_LOGE(TAG, "runWasm: Exception: %s\n", exception);
        return ESP_FAIL;
    }

    if (m->sp >= 0)
    {
        StackValue *result = &m->stack[m->sp--];
        switch (result->value_type)
        {
        case I32:
            ESP_LOGI(TAG, "I32 return value: 0x%x:i32", result->value.uint32);
            sprintf(fc->res, "%i", (int) result->value.uint32);
            break;
        case I64:
            ESP_LOGI(TAG, "I64 return value: 0x%llx:i64", result->value.uint64);
            sprintf(fc->res, "%lli", result->value.uint64);
            break;
        case F32:
            ESP_LOGI(TAG, "F32 return value: %.7g:f32", result->value.f32);
            sprintf(fc->res, "%f", (float) result->value.f32);
            break;
        case F64:
            ESP_LOGI(TAG, "F64 return value: %.7g:f64", result->value.f64);
            sprintf(fc->res, "%f:f64", (double) result->value.f64);
            break;
        }
    }
    else
    {
        ESP_LOGI(TAG, "runWasm: No result.\n");
    }
    ESP_LOGI(TAG, "\n\n ------ :-) DONE ------\n\n");
    return ESP_OK;
}

void parseWasm(unsigned char *bytes, size_t byte_count)
{
    Options opts;
    Module *m = load_module(bytes, byte_count, opts);
    m->path = "arith.wasm";

    init_thunk_in(m);
}
