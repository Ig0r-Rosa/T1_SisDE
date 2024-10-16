#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"        // Para semáforos e mutexes
#include "driver/gpio.h"
#include "driver/uart.h"            // Para uart_driver_install e UART_NUM_0
#include "esp_timer.h"
#include "esp_log.h"                // Para esp_log_level_set e ESP_LOG_INFO

// Definições
#define TRIG_PIN GPIO_NUM_22
#define ECHO_PIN GPIO_NUM_23
#define LOW 0
#define HIGH 1

// Variáveis globais
volatile float distanciaChao = -1.0f;
volatile float distanciaAtual = -1.0f;
volatile int escolha = -1;

// Mutex para proteger distanciaAtual
SemaphoreHandle_t distanciaMutex;

// Tags para logging
#define TAG_MENU "MenuTask"
#define TAG_DIST "DistanceTask"
#define TAG_ALTURA "ExibirAlturaTask"

// Funções
void Calibrar();
void MostrarAltura();
void CalcularDistancia(void *pvParameters);
void exibirAltura();

// Flag para controlar a exibição da altura
volatile bool exibindoAltura = false;

// Função para exibir o menu
void exibirMenu()
{
    printf("    Menu    \n"
           "Para realizar a calibragem você precisa instalar o "
           "sensor na posição final dele. Após, entrar com a "
           "opção 1 no menu.\n"
           "[1] - Captura a altura para calibragem.\n"
           "[2] - Mostra a altura capturada.\n"
           "[0] - Retorna para o menu.\n"
           "Escolha: ");
}

// Tarefa do Menu
void MenuTask(void *pvParameters)
{
    exibirMenu();

    while(1)
    {

        escolha = -1;

        scanf("%i", &escolha);

        switch (escolha)
        {
            case 0:
                exibindoAltura = false;
                exibirMenu();
                break;

            case 1:
                Calibrar();
                break;
            
            case 2:
                MostrarAltura();
                break;
            
            default:
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(1)); // Aguarda 1 segundo antes de exibir o menu novamente
    }
}

// Função para Calibrar
void Calibrar()
{
    float localDistAtual;

    // Adquire o mutex antes de ler distanciaAtual
    if(xSemaphoreTake(distanciaMutex, portMAX_DELAY) == pdTRUE) 
    {
        localDistAtual = distanciaAtual;
        xSemaphoreGive(distanciaMutex); // Libera o mutex após a leitura
    } 
    else 
    {
        // Handle error
        ESP_LOGE(TAG_MENU, "Erro ao adquirir o mutex.");
        printf("Erro ao adquirir o mutex.\n");
        return;
    }

    distanciaChao = localDistAtual;
    ESP_LOGI(TAG_MENU, "A altura do chão é %.2f metros.", distanciaChao);
    printf("\nA altura do chão é %.2f metros.\n\n", distanciaChao);
}

// Função para Mostrar a Altura
void MostrarAltura()
{
    if(exibindoAltura) 
    {
        // Se já estiver exibindo, não faz nada
        return;
    }

    exibindoAltura = true;

    // Cria a tarefa para exibir a altura
    xTaskCreate(
        exibirAltura,          // Função da tarefa
        "Exibir Altura Task",  // Nome da tarefa
        2048,                  // Tamanho da pilha
        NULL,                  // Parâmetro passado para a função
        2,                     // Prioridade da tarefa
        NULL                   // Handle da tarefa (não utilizado aqui)
    );
}

// Tarefa para exibir a altura
void exibirAltura()
{
    while(exibindoAltura)
    {
        float localDistChao, localDistAtual;

        // Adquire o mutex para ler distanciaChao e distanciaAtual
        if(xSemaphoreTake(distanciaMutex, portMAX_DELAY) == pdTRUE) 
        {
            localDistChao = distanciaChao;
            localDistAtual = distanciaAtual;
            xSemaphoreGive(distanciaMutex); // Libera o mutex após a leitura
        } 
        else 
        {
            // Handle error
            ESP_LOGE(TAG_ALTURA, "Erro ao adquirir o mutex.");
            printf("Erro ao adquirir o mutex.\n");
            return;
        }

        float altura = localDistChao - localDistAtual;
        if(altura > 0.35f)
        {
            ESP_LOGI(TAG_ALTURA, "A altura detectada é %.2f metros.", altura);
            printf("\nA altura detectada é %.2f metros.\n\n", altura);
        }

        vTaskDelay(pdMS_TO_TICKS(500)); // Aguarda 500ms antes de verificar novamente
    }

    // Deleta a tarefa após finalizar
    vTaskDelete(NULL);
}

// Tarefa para Calcular Distância
void CalcularDistancia(void *pvParameters)
{
    // Calcula a distância do sensor HC04
    while(1) 
    {
        // Envia pulso de trigger
        gpio_set_level(TRIG_PIN, HIGH);
        vTaskDelay(10); // Pulso de 10 microsegundos
        gpio_set_level(TRIG_PIN, LOW);

        // Aguarda o echo começar
        while(gpio_get_level(ECHO_PIN) == LOW) 
        {
            // Aguarda o echo começar
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        int64_t t1 = esp_timer_get_time();

        // Aguarda o echo acabar
        while(gpio_get_level(ECHO_PIN) == HIGH) 
        {
            // Aguarda o echo acabar
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        int64_t t2 = esp_timer_get_time();

        int64_t pulse_time = t2 - t1; // Tempo do pulso em microsegundos
        float distancia;

        // Calcula a distância em metros
        // Velocidade do som: ~343 m/s => ~0.0343 cm/µs
        // Fórmula: (tempo / 2) * 0.0343 cm/µs = distância em cm
        // Convertendo para metros:
        distancia = (pulse_time / 2.0f) * 0.0343f / 100.0f; // Distância em metros

        // Log dos valores para depuração
        ESP_LOGI(TAG_DIST, "Pulse Time: %lld us, Distância: %.2f metros", pulse_time, distancia);

        // Valida a distância
        if (distancia >= 4.0f || distancia <= 0.05f)
        {
            // Fora do alcance
            ESP_LOGI(TAG_DIST, "Fora do alcance: %.2f metros", distancia);
            distancia = -1.0f; // Valor inválido
        }

        // Adquire o mutex antes de escrever em distanciaAtual
        if(xSemaphoreTake(distanciaMutex, portMAX_DELAY) == pdTRUE) 
        {
            distanciaAtual = distancia;
            xSemaphoreGive(distanciaMutex); // Libera o mutex após a escrita
        } 
        else 
        {
            // Handle error
            ESP_LOGE(TAG_DIST, "Erro ao adquirir o mutex.");
            printf("Erro ao adquirir o mutex.\n");
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda 1 segundo antes da próxima medição
    }
}

// Função principal do aplicativo
void app_main(void)
{
    // Inicializa a comunicação serial
    uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0);
    esp_log_level_set("*", ESP_LOG_INFO);

    // Cria o mutex
    distanciaMutex = xSemaphoreCreateMutex();
    if (distanciaMutex == NULL) {
        ESP_LOGE(TAG_MENU, "Erro ao criar o mutex.");
        printf("Erro ao criar o mutex.\n");
        return;
    }

    // Configura os pinos do sensor HC-SR04
    gpio_reset_pin(TRIG_PIN); // trig
    gpio_reset_pin(ECHO_PIN); // echo
    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);

    // Inicializa os pinos
    gpio_set_level(TRIG_PIN, LOW); // Assegura que o trigger está baixo inicialmente

    // Criação das tarefas
    // Tarefa para o menu rodando no Core 0
    xTaskCreatePinnedToCore(
        MenuTask,               // Função da tarefa
        "Menu Task",            // Nome da tarefa
        4096,                   // Tamanho da pilha
        NULL,                   // Parâmetro passado para a função
        1,                      // Prioridade da tarefa
        NULL,                   // Handle da tarefa (não utilizado aqui)
        0                       // Núcleo 0
    );

    // Tarefa para calcular a distância rodando no Core 1
    xTaskCreatePinnedToCore(
        CalcularDistancia,      // Função da tarefa
        "Distance Task",        // Nome da tarefa
        4096,                   // Tamanho da pilha
        NULL,                   // Parâmetro passado para a função
        1,                      // Prioridade da tarefa
        NULL,                   // Handle da tarefa (não utilizado aqui)
        1                       // Núcleo 1
    );
}
