/*
 * Wasm3 - high performance WebAssembly interpreter written in C.
 * Copyright © 2020 Volodymyr Shymanskyy, Steven Massey.
 * All rights reserved.
 */

#include <wasm3.h>

#include <m3_api_defs.h>
#include <m3_env.h>

#include <Arduino.h>

/*
 * Configuration
 */

#define WASM_STACK_SLOTS 2048
#define NATIVE_STACK_SIZE (32 * 1024)

// For (most) devices that cannot allocate a 64KiB wasm page
//#define WASM_MEMORY_LIMIT   4096

/*
 * WebAssembly app
 */

#include "../app/app.wasm.h"

/*
 * API bindings
 *
 * Note: each RawFunction should complete with one of these calls:
 *   m3ApiReturn(val)   - Returns a value
 *   m3ApiSuccess()     - Returns void (and no traps)
 *   m3ApiTrap(trap)    - Returns a trap
 */

m3ApiRawFunction(m3_arduino_millis)
{
    m3ApiReturnType(uint32_t)

        m3ApiReturn(millis());
}

m3ApiRawFunction(m3_arduino_delay)
{
    m3ApiGetArg(uint32_t, ms)

        // You can also trace API calls
        //Serial.print("api: delay "); Serial.println(ms);

        delay(ms);

    m3ApiSuccess();
}

m3ApiRawFunction(m3_arduino_print)
{
    m3ApiGetArgMem(const uint8_t *, buf)
        m3ApiGetArg(uint32_t, len)

        //printf("api: print %p %d\n", buf, len);
        Serial.write(buf, len);

    m3ApiSuccess();
}

M3Result LinkArduino(IM3Runtime runtime)
{
    IM3Module module = runtime->modules;
    const char *arduino = "arduino";

    m3_LinkRawFunction(module, arduino, "millis", "i()", &m3_arduino_millis);
    m3_LinkRawFunction(module, arduino, "delay", "v(i)", &m3_arduino_delay);
    m3_LinkRawFunction(module, arduino, "print", "v(*i)", &m3_arduino_print);

    return m3Err_none;
}

/*
 * Engine start, liftoff!
 */

#define FATAL(func, msg)                  \
    {                                     \
        Serial.print("Fatal: " func " "); \
        Serial.println(msg);              \
        return;                           \
    }

void wasm_task(void *)
{
    M3Result result = m3Err_none;

    IM3Environment env = m3_NewEnvironment();
    if (!env)
        FATAL("NewEnvironment", "failed");

    IM3Runtime runtime = m3_NewRuntime(env, WASM_STACK_SLOTS, NULL);
    if (!runtime)
        FATAL("NewRuntime", "failed");

#ifdef WASM_MEMORY_LIMIT
    runtime->memoryLimit = WASM_MEMORY_LIMIT;
#endif

    IM3Module module;
    result = m3_ParseModule(env, &module, app_wasm, app_wasm_len - 1);
    if (result)
        FATAL("ParseModule", result);

    result = m3_LoadModule(runtime, module);
    if (result)
        FATAL("LoadModule", result);

    result = LinkArduino(runtime);
    if (result)
        FATAL("LinkArduino", result);

    IM3Function f;
    result = m3_FindFunction(&f, runtime, "_start");

    if (result)
        FATAL("FindFunction", result);

    Serial.println("Running WebAssembly...");

    const char *i_argv[1] = {NULL};
    result = m3_CallWithArgs(f, 0, i_argv);

    // Should not arrive here

    if (result)
    {
        M3ErrorInfo info;
        m3_GetErrorInfo(runtime, &info);
        Serial.print("Error: ");
        Serial.print(result);
        Serial.print(" (");
        Serial.print(info.message);
        Serial.println(")");
    }
}

void setup()
{
    Serial.begin(115200);
    delay(100);

    // Wait for serial port to connect
    // Needed for native USB port only
    while (!Serial)
    {
    }

    Serial.println("\nWasm3 v" M3_VERSION ", build " __DATE__ " " __TIME__);

    xTaskCreate(&wasm_task, "wasm3", NATIVE_STACK_SIZE, NULL, 5, NULL);
}

void loop()
{
    delay(100);
}
