#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define LOW 0
#define HIGH 1
#define TRIG_PIN 22
#define ECHO_PIN 23

void weight(void *arg)
{
    int64_t t1 = 0, t2 = 0, pulse_time = 0;
    float distance;
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
        distance = (pulse_time/2) * 0.0344;

        if (distance >= 400 || distance <= 5)
        {
            printf("Out of range");
        }
        else
        {
            printf("Distância medida: %11f\n", distance);
        }

        vTaskDelay(1000/ portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    gpio_reset_pin(TRIG_PIN); // trig
    gpio_reset_pin(ECHO_PIN); // echo
    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);

    xTaskCreate(weight, "altura", 2048, NULL, 1, NULL);

}
