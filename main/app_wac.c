
#include "app_wac.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <esp_log.h>

// Call table/trapping table lookups/execution
#include <unistd.h>
#include <signal.h>

#include "util.h"
#include "platform.h"
#include "wa.h"
#include "thunk.h"

#include "app_wifi_http.h"

const char* TAG = "app_wac: ";

// WASM test files

unsigned char arith_wasm[] = {
    0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x07, 0x01, 0x60,
    0x02, 0x7f, 0x7f, 0x01, 0x7f, 0x03, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00,
    0x07, 0x19, 0x04, 0x03, 0x61, 0x64, 0x64, 0x00, 0x00, 0x03, 0x73, 0x75,
    0x62, 0x00, 0x01, 0x03, 0x6d, 0x75, 0x6c, 0x00, 0x02, 0x03, 0x64, 0x69,
    0x76, 0x00, 0x03, 0x0a, 0x21, 0x04, 0x07, 0x00, 0x20, 0x00, 0x20, 0x01,
    0x6a, 0x0b, 0x07, 0x00, 0x20, 0x00, 0x20, 0x01, 0x6b, 0x0b, 0x07, 0x00,
    0x20, 0x00, 0x20, 0x01, 0x6c, 0x0b, 0x07, 0x00, 0x20, 0x00, 0x20, 0x01,
    0x6e, 0x0b};
unsigned int arith_wasm_len = 86;

const char *arith_wasm_funcs[] = {
    "add",
    "sub"
    "mul",
    "div"};

unsigned char wasi_test_wasm[] = {
    0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x09, 0x02, 0x60,
    0x00, 0x00, 0x60, 0x01, 0x7f, 0x01, 0x7f, 0x03, 0x04, 0x03, 0x00, 0x01,
    0x01, 0x04, 0x05, 0x01, 0x70, 0x01, 0x01, 0x01, 0x05, 0x03, 0x01, 0x00,
    0x02, 0x06, 0x15, 0x03, 0x7f, 0x01, 0x41, 0x80, 0x88, 0x04, 0x0b, 0x7f,
    0x00, 0x41, 0x80, 0x88, 0x04, 0x0b, 0x7f, 0x00, 0x41, 0x80, 0x08, 0x0b,
    0x07, 0x34, 0x05, 0x06, 0x6d, 0x65, 0x6d, 0x6f, 0x72, 0x79, 0x02, 0x00,
    0x0b, 0x5f, 0x5f, 0x68, 0x65, 0x61, 0x70, 0x5f, 0x62, 0x61, 0x73, 0x65,
    0x03, 0x01, 0x0a, 0x5f, 0x5f, 0x64, 0x61, 0x74, 0x61, 0x5f, 0x65, 0x6e,
    0x64, 0x03, 0x02, 0x03, 0x66, 0x69, 0x62, 0x00, 0x01, 0x06, 0x66, 0x69,
    0x62, 0x52, 0x65, 0x63, 0x00, 0x02, 0x0a, 0x5c, 0x03, 0x02, 0x00, 0x0b,
    0x2e, 0x01, 0x03, 0x7f, 0x41, 0x01, 0x21, 0x01, 0x20, 0x00, 0x41, 0x01,
    0x6a, 0x21, 0x00, 0x41, 0x00, 0x21, 0x02, 0x03, 0x40, 0x20, 0x01, 0x22,
    0x03, 0x20, 0x02, 0x6a, 0x21, 0x01, 0x20, 0x03, 0x21, 0x02, 0x20, 0x00,
    0x41, 0x7f, 0x6a, 0x22, 0x00, 0x0d, 0x00, 0x0b, 0x20, 0x01, 0x0b, 0x28,
    0x00, 0x02, 0x40, 0x02, 0x40, 0x20, 0x00, 0x45, 0x0d, 0x00, 0x20, 0x00,
    0x41, 0x01, 0x47, 0x0d, 0x01, 0x41, 0x01, 0x0f, 0x0b, 0x41, 0x00, 0x0f,
    0x0b, 0x20, 0x00, 0x41, 0x7f, 0x6a, 0x10, 0x82, 0x80, 0x80, 0x80, 0x00,
    0x20, 0x00, 0x6a, 0x0b, 0x00, 0x28, 0x04, 0x6e, 0x61, 0x6d, 0x65, 0x01,
    0x21, 0x03, 0x00, 0x11, 0x5f, 0x5f, 0x77, 0x61, 0x73, 0x6d, 0x5f, 0x63,
    0x61, 0x6c, 0x6c, 0x5f, 0x63, 0x74, 0x6f, 0x72, 0x73, 0x01, 0x03, 0x66,
    0x69, 0x62, 0x02, 0x06, 0x66, 0x69, 0x62, 0x52, 0x65, 0x63};
unsigned int wasi_test_wasm_len = 250;

const char *wasi_test_wasm_funcs[] = {
    "fib",
    "fibRec"};

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

Memory _env__memory_ = {
    PAGE_COUNT,  // initial size (64K pages)
    TOTAL_PAGES, // max size (64K pages)
    PAGE_COUNT,  // current size (64K pages)
    NULL};       // memory base
uint8_t *_env__memoryBase_;

Table _env__table_ = {
    ANYFUNC,     // on;y allowed value in WASM MVP
    TABLE_COUNT, // initial
    TABLE_COUNT, // max
    TABLE_COUNT, // current
    0};
//uint32_t *_env__table_ = 0;
uint32_t *_env__tableBase_;

double _global__NaN_ = NAN;
double _global__Infinity_ = INFINITY;

uint32_t **_env__DYNAMICTOP_PTR_;
uint32_t *_env__tempDoublePtr_;

bool wacIsInitalized = false;

/** 
 * Initializw the WAC Interpreter part 
 *
 *  mainly initialize memory globals
 *
 */
void initializeWac()
{
    // lazy initialize at first use of the Interpreter
    if (wacIsInitalized) return;
    wacIsInitalized = true;

    _env__memoryBase_ = calloc(PAGE_COUNT, PAGE_SIZE);

    _env__tempDoublePtr_ = (uint32_t *)_env__memoryBase_;
    _env__DYNAMICTOP_PTR_ = (uint32_t **)(_env__memoryBase_ + 16);

    *_env__DYNAMICTOP_PTR_ = (uint32_t *)(_env__memoryBase_ + PAGE_COUNT * PAGE_SIZE);

    // This arrangement correlates to the module mangle_table_offset option
    //    if (posix_memalign((void **)&_env__table_.entries, sysconf(_SC_PAGESIZE),
    //                       TABLE_COUNT*sizeof(uint32_t))) {
    //            perror("posix_memalign");
    _env__table_.entries = malloc(TABLE_COUNT * sizeof(uint32_t));
    if (_env__table_.entries == NULL)
    {
        //
        perror("InitializeWac: malloc(tablecount....)");
        exit(1);
    }
    _env__tableBase_ = _env__table_.entries;

    info("init_mem results:\n");
    info("  _env__memory_.bytes: %p\n", _env__memory_.bytes);
    info("  _env__memoryBase_: %p\n", _env__memoryBase_);
    info("  _env__DYNAMIC_TOP_PTR_: %p\n", _env__DYNAMICTOP_PTR_);
    info("  *_env__DYNAMIC_TOP_PTR_: %p\n", *_env__DYNAMICTOP_PTR_);
    info("  _env__table_.entries: %p\n", _env__table_.entries);
    info("  _env__tableBase_: 0x%x\n", (unsigned int)_env__tableBase_);
}


void runWasm() {
    Module *m = getModule();

    if (m == NULL) {
        ESP_LOGW(TAG, "runWasm - no module loaded");
        // FIXME - send back error result
    }

    // setup argc/argv
    m->stack[++m->sp].value_type = I32;
    m->stack[m->sp].value.uint32 = 3;
    //   m->stack[m->sp].value.uint32 = 361;  // mac result that fits to two bytes

    // Invoke main/_main function and exit
    int fidx = get_export_fidx(m, "fibRec");
    if (fidx == -1)
    {
        fidx = get_export_fidx(m, "fib");
        if (fidx == -1)
        {
            FATAL("no exported function named 'fib' or 'finbRec'\n");
        }
    }
    int res = invoke(m, fidx);

    if (!res)
    {
        error("Exception: %s\n", exception);
        return;
    }

    if (m->sp >= 0)
    {
        StackValue *result = &m->stack[m->sp--];
        switch (result->value_type)
        {
        case I32:
            printf("I32 return value: 0x%x:i32", result->value.uint32);
            break;
        case I64:
            printf("I64 return value: 0x%llx:i64", result->value.uint64);
            break;
        case F32:
            printf("F32 return value: %.7g:f32", result->value.f32);
            break;
        case F64:
            printf("F64 return value: %.7g:f64", result->value.f64);
            break;
        }
        // value_repr(&m->stack[m->sp--]);
    }
    else
    {
        printf("No result.\n");
    }
    printf("\n\n ------ :-) DONE ------\n\n");
}


void parseWasm(unsigned char *bytes, size_t byte_count)
{
    // lazy init if not done earlier
    initializeWac();

    Options opts;
    Module *m = load_module(bytes, byte_count, opts);
    m->path = "arith.wasm";

    init_thunk_in(m);
    free(bytes);
}

