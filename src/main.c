// SPDX-License-Identifier: MIT

#include <stdio.h>

#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"

void vTaskHello(void *pvParameters);
void vTaskWorld(void *pvParameters);

int main(void) {
    // Initialize clock
    SystemCoreClockUpdate();

    /* Initialize UART for libc function calls. */
    USART_Printf_Init(115200);

    printf("\n#------------------------------------------------#\n\n");

    xTaskCreate(vTaskHello, "HELLO", 256, NULL, 4, NULL);
    xTaskCreate(vTaskWorld, "WORLD", 256, NULL, 4, NULL);

    vTaskStartScheduler();

    for (;;) {
        /* Never reaches here. */
    }
}

void vTaskHello(void *pvParameters) {
    for (;;) {
        printf("Hello\n");
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
void vTaskWorld(void *pvParameters)
{
    for (;;)
    {
        printf("World\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}