#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_timer.h"

// Definições
#define LOW 0
#define HIGH 1
#define TRIG_PIN 22
#define ECHO_PIN 23

// Variaveis globais
volatile int tempoTriggerMili = 10;
volatile float distanciaChao = -1;
volatile float distanciaAtual = -1;
volatile int duracao = -1;
volatile int escolha = -1;

void Calibrar();
void MostrarAltura();
void CalcularDistancia();

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
    distanciaChao = distanciaAtual;
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

void CalcularDistancia()
{
    // Calcula a distancia do sensor HC04
    int64_t t1 = 0, t2 = 0, pulse_time = 0;
    while(1) 
    {
        gpio_set_level(TRIG_PIN, HIGH);
        vTaskDelay(10 / portTICK_PERIOD_MS);
        gpio_set_level(TRIG_PIN, LOW);

        while(gpio_get_level(ECHO_PIN) == LOW)
        {
            // aguarda o echo começar
        }

        t1 = esp_timer_get_time();

        while(gpio_get_level(ECHO_PIN) == HIGH)
        {
            // aguarda echo acabar
        }

        t2 = esp_timer_get_time();

        pulse_time = t2 - t1;
        distanciaAtual = (pulse_time/2) * 0.0344;

        if (distanciaAtual >= 400 || distanciaAtual <= 5)
        {
            printf("Out of range");
        }

        vTaskDelay(1000/ portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    // Pinos
    gpio_reset_pin(TRIG_PIN); // trig
    gpio_reset_pin(ECHO_PIN); // echo
    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);

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
        CalcularDistancia,   // Função da tarefa
        "Distance Task",     // Nome da tarefa
        4096,                // Tamanho da pilha
        NULL,                // Parâmetro passado para a função
        1,                   // Prioridade da tarefa
        NULL,                // Handle da tarefa (não precisamos aqui)
        1                    // Núcleo 1
    );
}
