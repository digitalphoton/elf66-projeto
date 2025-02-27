
/* -------------------------------------------------------------------------- */
/* Includes de bibliotecas                                                    */
/* -------------------------------------------------------------------------- */

#include <stdio.h>

#include "debug.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* -------------------------------------------------------------------------- */
/*   Definições de tipo                                                       */
/* -------------------------------------------------------------------------- */

typedef enum
{
    STARTUP,
    STANDBY,
    DESLIGADO,
    PISCA,
    LIGADO
}
Estados;

typedef enum
{
    NIVEL_0,
    NIVEL_1,
    NIVEL_2,
    NIVEL_3,
    NIVEL_4
}
Niveis;

/* -------------------------------------------------------------------------- */
/*   Variáveis Globais                                                        */
/* -------------------------------------------------------------------------- */

Estados g_estadoAtual = STARTUP;
Niveis g_nivelAtual = NIVEL_0;
uint16_t g_valorPot = 0x00ff;

SemaphoreHandle_t g_semaforo;

/* -------------------------------------------------------------------------- */
/*   Protótipos de funções                                                    */
/* -------------------------------------------------------------------------- */

void usart1Begin(uint32_t baudrate);
void gpioBegin(void);

void delay(uint64_t milliseconds);

void taskSensor(void *pvParameters);
void taskAtuador(void *pvParameters);
void taskEnvia(void *pvParameters);
void taskRecebe(void *pvParameters);

/* -------------------------------------------------------------------------- */
/* Função main()                                                              */
/* -------------------------------------------------------------------------- */

int main(void)
{
    // Initialize clock
    SystemCoreClockUpdate();

    gpioBegin();
    usart1Begin(115200);

    printf("\n#------------------------------------------------#\n\n");
    
    g_semaforo = xSemaphoreCreateMutex();

    xTaskCreate(taskSensor, "Sensor", 256, NULL, 4, NULL);
    xTaskCreate(taskAtuador, "Atuador", 256, NULL, 1, NULL);
    xTaskCreate(taskEnvia, "Envia", 256, NULL, 1, NULL);
    xTaskCreate(taskRecebe, "Recebe", 256, NULL, 4, NULL);

    g_estadoAtual = PISCA;

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
    /* Ativar o clock da USART1 */
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, ENABLE );

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

void gpioBegin(void)
{
    /* Ativar o clock do GPIOA e do GPIOB */

    RCC_APB2PeriphClockCmd
    (
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB,
        ENABLE
    );

    /* Inicializar os pinos PA9 (TX) e PA10 (RX) do GPIOA como função alternativa, com saída push-pull */

    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Inicializar o pino PB13 como pino digital de saída push-pull */

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void delay(uint64_t milliseconds)
{
    for(uint64_t i = milliseconds; i > 0; i--)
    {
        vTaskDelay( pdMS_TO_TICKS(1) );
    }
}

/* -------------------------------------------------------------------------- */
/*   Declaração das Tasks                                                     */
/* -------------------------------------------------------------------------- */

void taskSensor(void *pvParameters)
{
    for (;;)
    {
        /* Lê o valor do potenciometro e coloca em uma variável global */

        /* Pegamos um semaforo para garantir acesso exclusivo às variáveis globais */
        if(xSemaphoreTake( g_semaforo, (TickType_t)(10) ) == pdTRUE)
        {
            switch(g_nivelAtual)
            {
                default:
                case NIVEL_0:
                {
                    g_valorPot = 0;
                    break;
                }
                case NIVEL_1:
                {
                    g_valorPot = 0x3ff;
                    break;
                }
                case NIVEL_2:
                {
                    g_valorPot = 0x7ff;
                    break;
                }
                case NIVEL_3:
                {
                    g_valorPot = 0xbff;
                    break;
                }
                case NIVEL_4:
                {
                    g_valorPot = 0xfff;
                    break;
                }
            }
            /* Devolvemos o semáforo, liberando o acesso às demais tasks */
            xSemaphoreGive(g_semaforo);
        }

        vTaskDelay(pdMS_TO_TICKS(5000));

        g_nivelAtual++;
        if(g_nivelAtual > NIVEL_4)
        {
            g_nivelAtual = NIVEL_0;
        }
    }
}
void taskAtuador(void *pvParameters)
{
    for (;;)
    {
        switch(g_estadoAtual)
        {
            default:
            case DESLIGADO:
            {
                /* desligar LED (lógica invertida) */
                GPIO_WriteBit(GPIOB, GPIO_Pin_13, Bit_SET);
                vTaskDelay(pdMS_TO_TICKS(10));
                break;
            }
            case LIGADO:
            {
                /* ligar LED (lógica invertida) */
                GPIO_WriteBit(GPIOB, GPIO_Pin_13, Bit_RESET);
                vTaskDelay(pdMS_TO_TICKS(10));
                break;
            }
            case PISCA:
            {
                /* piscar LED */
                GPIO_WriteBit(GPIOB, GPIO_Pin_13, Bit_RESET);
                delay(g_valorPot);
                GPIO_WriteBit(GPIOB, GPIO_Pin_13, Bit_SET);
                delay(g_valorPot);
                break;
            }
        }
    }
}
void taskEnvia(void *pvParameters)
{
    for (;;)
    {
        /* Envia o estado atual e o valor da variável global do sensor para o PC */
        printf("Estado atual: ");
        switch(g_estadoAtual)
        {
            default:
            {
                printf("INVÁLIDO");
                break;
            }
            case STANDBY:
            {
                printf("STANDBY");
                break;
            }
            case STARTUP:
            {
                printf("STARTUP");
                break;
            }
            case DESLIGADO:
            {
                printf("DESLIGADO");
                break;
            }
            case LIGADO:
            {
                printf("LIGADO");
                break;
            }
            case PISCA:
            {
                printf("PISCA");
                break;
            }
        }
        printf(" ; Valor do Potenciometro: %d\n", g_valorPot);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void taskRecebe(void *pvParameters)
{
    for (;;)
    {
        /* Recebe comandos para mudar o estado atual */
        if( USART_GetFlagStatus( USART1, USART_FLAG_RXNE ) )
        {
            char dado;
            dado = (uint8_t)(USART_ReceiveData(USART1) & 0x00ff);
        
            /* Pegamos um semaforo para garantir acesso exclusivo às variáveis globais */
            if( xSemaphoreTake( g_semaforo, (TickType_t)(10) ) == pdTRUE )
            {
                switch(dado)
                {
                    default:
                    case '0':
                    {
                        g_nivelAtual = NIVEL_0;
                        g_valorPot = 0x000;
                        break;
                    }
                    case '1':
                    {
                        g_nivelAtual = NIVEL_1;
                        g_valorPot = 0x3ff;
                        break;
                    }
                    case '2':
                    {
                        g_nivelAtual = NIVEL_2;
                        g_valorPot = 0x7ff;
                        break;
                    }
                    case '3':
                    {
                        g_nivelAtual = NIVEL_3;
                        g_valorPot = 0xbff;
                        break;
                    }
                    case '4':
                    {
                        g_nivelAtual = NIVEL_4;
                        g_valorPot = 0xfff;
                        break;
                    }
                }

                /* Devolvemos o semáforo, liberando o acesso às demais tasks */
                xSemaphoreGive(g_semaforo);
            }
        }
        vTaskDelay(1);
    }
}