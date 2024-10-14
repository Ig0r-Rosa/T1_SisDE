#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

// Variaveis globais
volatile int tempoTriggerMili = 10;
volatile float distanciaChao = -1;
volatile float distanciaAtual = -1;
volatile int duracao = -1;
volatile int escolha = -1;

void Calibrar();
void MostrarAltura();
float CalcularDistancia();

void Menu()
{
    while(1)
    {
        printf( "    Menu    \n"
                "Para realizar a calibragem você precisa instalar o"
                "sensor na posição final dele. Após, entrar com a"
                "opção 1 no menu.\n"
                "[1] - Captura a altura para calibragem. \n"
                "[2] - Mostra a altura capturada. \n"
                "[0] - Retorna para o o menu. \n"
                "Escolha: ");

        scanf("%i", &escolha);

        switch (escolha)
        {
        case 1:
            Calibrar();
            break;
        
        case 2:
            MostrarAltura();
            break;
        
        default:
            break;
        }

        vTaskDelay(10);
    }
}

void Calibrar()
{
    distanciaChao = CalcularDistancia();
    printf("\n A altura do chão é %.2f metros. \n\n", distanciaChao);
}

void MostrarAltura()
{
    if(distanciaChao - distanciaAtual > 0.35f)
    {
        printf("\n A altura detectada é %.2f metros. \n\n", distanciaChao - distanciaAtual);
    }
    vTaskDelay(50);
}

float CalcularDistancia()
{
    // Calcula a distancia do sensor HC04
    return -1;
}

// Função que fica verificando continuamente a distância
void VerificaDistancia(void *pvParameters)
{
    while(1)
    {
        distanciaAtual = CalcularDistancia();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    // Pinos
    gpio_num_t Echo = GPIO_NUM_22;
    gpio_num_t Trigger = GPIO_NUM_23;

    /* Set the GPIO as a input */
    gpio_install_isr_service(1);

    gpio_set_direction(Echo, GPIO_MODE_INPUT);
    gpio_set_direction(Trigger, GPIO_MODE_OUTPUT);

    /* Set the GPIO pull */
    gpio_set_pull_mode(Echo, GPIO_PULLUP_ONLY);

    // Chamada das threads
    // Tarefa para o menu rodando no Core 0
    xTaskCreatePinnedToCore(
        Menu,               // Função da tarefa
        "Menu Task",        // Nome da tarefa
        4096,               // Tamanho da pilha
        NULL,               // Parâmetro passado para a função
        1,                  // Prioridade da tarefa
        NULL,               // Handle da tarefa (não precisamos aqui)
        0                   // Núcleo 0
    );

    // Tarefa para verificar a distância rodando no Core 1
    xTaskCreatePinnedToCore(
        VerificaDistancia,   // Função da tarefa
        "Distance Task",     // Nome da tarefa
        4096,                // Tamanho da pilha
        NULL,                // Parâmetro passado para a função
        1,                   // Prioridade da tarefa
        NULL,                // Handle da tarefa (não precisamos aqui)
        1                    // Núcleo 1
    );
}
