
/* -------------------------------------------------------------------------- */
/* Includes de bibliotecas                                                    */
/* -------------------------------------------------------------------------- */

#include <stdio.h>

#include "ch32v30x_adc.h"

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

int16_t g_calibracaoADC = 0;

SemaphoreHandle_t g_semaforo;

/* -------------------------------------------------------------------------- */
/*   Protótipos de funções                                                    */
/* -------------------------------------------------------------------------- */

void usart1Begin(uint32_t baudrate);
void gpioBegin(void);
void adcBegin(void);

uint16_t getConvVal(uint8_t channel);
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
    adcBegin();

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

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /* Inicializar o pino PA0 como entrada analógica */

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Inicializar os pinos PA9 (TX) e PA10 (RX) do GPIOA como função alternativa, com saída push-pull */

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Inicializar o pino PB13 como pino digital de saída push-pull */

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

    GPIO_Init(GPIOB, &GPIO_InitStructure);
}
void adcBegin(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    ADC_DeInit(ADC1);

    ADC_InitTypeDef ADC_InitStructure;

    /* Inicializar o ADC1 no modo independente, sem scan, sem conversão contínua, sem trigger externo, com dados alinhados à direita (conversão ocupa os 12 bits menos significativos do registrador de saída), com um canal apenas, sem buffer de saída, e com ganho unitário na entrada */

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = DISABLE;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_InitStructure.ADC_OutputBuffer = ADC_OutputBuffer_Disable;
    ADC_InitStructure.ADC_Pga = ADC_Pga_1;

    ADC_Init(ADC1, &ADC_InitStructure);
    ADC_Cmd(ADC1, ENABLE);

    /* Executar rotina de autocalibração do ADC1 */

    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));

    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
    g_calibracaoADC = Get_CalibrationValue(ADC1);
}

uint16_t getConvVal(uint8_t channel)
{
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
    ADC_Cmd(ADC1, ENABLE);

    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    uint16_t adcVal = ADC_GetConversionValue(ADC1);

    if(adcVal + g_calibracaoADC < 0 || adcVal <= 0)
    {
        return 0;
    }
    if(adcVal + g_calibracaoADC > 0xfff || adcVal >= 0xfff)
    {
        return 0xfff;
    }
    return adcVal + g_calibracaoADC;

}
void delay(uint64_t milliseconds)
{
    for(uint64_t i = milliseconds; i > 0; i--)
    {
        vTaskDelay( pdMS_TO_TICKS(1) );
    }
}

/* -------------------------------------------------------------------------- */
/*   Definição das Tasks                                                      */
/* -------------------------------------------------------------------------- */

void taskSensor(void *pvParameters)
{
    for (;;)
    {
        /* Lê o valor do potenciometro (canal 1) e coloca em uma variável global */
        uint16_t valorLido = getConvVal(ADC_Channel_0);

        /* Pegamos um semaforo para garantir acesso exclusivo às variáveis globais */
        if(xSemaphoreTake( g_semaforo, (TickType_t)(10) ) == pdTRUE)
        {
            g_valorPot = valorLido;
            /* Devolvemos o semáforo, liberando o acesso às demais tasks */
            xSemaphoreGive(g_semaforo);
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
void taskAtuador(void *pvParameters)
{
    for (;;)
    {
        Estados estadoAtual;
        uint16_t valorPot;
        /* Pegamos um semaforo para garantir acesso exclusivo às variáveis globais */
        if(xSemaphoreTake( g_semaforo, (TickType_t)(10) ) == pdTRUE)
        {
            estadoAtual = g_estadoAtual;
            valorPot = g_valorPot;
            /* Devolvemos o semáforo, liberando o acesso às demais tasks */
            xSemaphoreGive(g_semaforo);
        }

        switch(estadoAtual)
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
                GPIO_WriteBit(GPIOB, GPIO_Pin_13, Bit_SET);
                delay(valorPot);
                GPIO_WriteBit(GPIOB, GPIO_Pin_13, Bit_RESET);
                delay(valorPot);
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
        Estados estadoAtual;
        uint16_t valorPot;

        /* Pegamos um semaforo para poder acessar as variáveis globais, e então fazemos uma cópia local delas */
        if(xSemaphoreTake(g_semaforo, (TickType_t)(10)) == pdTRUE)
        {
            estadoAtual = g_estadoAtual;
            valorPot = g_valorPot;
            xSemaphoreGive(g_semaforo);
            /* Devolvemos o semáforo, liberando o acesso às demais tasks */
        }

        printf("Estado atual: ");
        switch(estadoAtual)
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
        printf(" ; Valor do Potenciometro: %d\n", valorPot);
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

            Estados novoEstado;
            switch(dado)
            {
                default:
                case 'd':
                case 'D':
                {
                    novoEstado = DESLIGADO;
                    break;
                }
                case 'p':
                case 'P':
                {
                    novoEstado = PISCA;
                    break;
                }
                case 'l':
                case 'L':
                {
                    novoEstado = LIGADO;
                    break;
                }
            }
        
            /* Pegamos um semaforo para garantir acesso exclusivo às variáveis globais */
            if( xSemaphoreTake( g_semaforo, (TickType_t)(10) ) == pdTRUE )
            {
                g_estadoAtual = novoEstado;
                /* Devolvemos o semáforo, liberando o acesso às demais tasks */
                xSemaphoreGive(g_semaforo);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}