// SPDX-License-Identifier: MIT

#include <stdio.h>

#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"

const char *compile_dt = __DATE__ " " __TIME__;

void vTaskHello(void *pvParameters);

int main(void) {
    /* Initialize board components. */
    SystemCoreClockUpdate();

    /* Initialize UART for libc function calls. */
    USART_Printf_Init(115200);

    printf("FreeRTOS demo compiled AT %s\r\n", compile_dt);

    xTaskCreate(vTaskHello, "HELLO", 256, NULL, 4, NULL);

    vTaskStartScheduler();

    for (;;) {
        /* Never reaches here. */
    }
}

void vTaskHello(void *pvParameters) {
    for (;;) {
        printf("Hello world? @%lu\r\n", xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}