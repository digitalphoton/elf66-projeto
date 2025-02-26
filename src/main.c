
/* -------------------------------------------------------------------------- */
/* Includes de bibliotecas                                                    */
/* -------------------------------------------------------------------------- */

#include <stdio.h>

#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"

/* -------------------------------------------------------------------------- */
/* Protótipos de funções                                                      */
/* -------------------------------------------------------------------------- */

void usart1Begin(uint32_t baudrate);

void vTaskHello(void *pvParameters);
void vTaskWorld(void *pvParameters);

/* -------------------------------------------------------------------------- */
/* Função main()                                                              */
/* -------------------------------------------------------------------------- */

int main(void) {
    // Initialize clock
    SystemCoreClockUpdate();

    /* Initialize UART for libc function calls. */
    usart1Begin(115200);
    

    printf("\n#------------------------------------------------#\n\n");

    xTaskCreate(vTaskHello, "HELLO", 256, NULL, 4, NULL);
    xTaskCreate(vTaskWorld, "WORLD", 256, NULL, 4, NULL);

    vTaskStartScheduler();

    for (;;) {
        /* Never reaches here. */
    }
}

/* -------------------------------------------------------------------------- */
/* Definição das funções                                                      */
/* -------------------------------------------------------------------------- */

void usart1Begin(uint32_t baudrate)
{
    /* Ativar o clock da USART1 e do GPIOA */

    RCC_APB2PeriphClockCmd
    (
        RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA,
        ENABLE
    );

    /* Inicializar os pinos PA9 (TX) e PA10 (RX) do GPIOA como função alternativa, com saída push-pull */

    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Inicializar a interface USART1 de acordo com a baudrate desejada, com 8 bits de dados, 1 bit de stop, sem paridade, sem controle de fluxo, e com transmissão e recepção */

    USART_InitTypeDef USART_InitStructure;

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

void vTaskHello(void *pvParameters) {
    for (;;)
    {
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