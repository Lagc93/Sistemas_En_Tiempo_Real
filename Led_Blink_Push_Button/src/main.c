#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_system.h>
#include "sdkconfig.h"

SemaphoreHandle_t xMutex; // Objeto de semaforo
bool stopLED = false; // Bandera para detener el parpadeo del LED

void tareaLP(void *arg)
{
    while (1)
    {
        //printf("Task\n");
        xSemaphoreTake(xMutex, portMAX_DELAY);
        if (!stopLED)
        {
            for (uint8_t i = 0; i < 10; i++)
            {
                gpio_set_level(GPIO_NUM_2, 1);
                esp_rom_delay_us(150);
                gpio_set_level(GPIO_NUM_2, 0);
                esp_rom_delay_us(150);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Tiempo de bloqueo de la tareaHP
        xSemaphoreGive(xMutex);
    }
}

void tareaHP(void *arg)
{
    while (1)
    {
        //printf("Task\n");
        xSemaphoreTake(xMutex, portMAX_DELAY);
        if (!stopLED)
        {
            for (uint8_t i = 0; i < 10; i++)
            {
                gpio_set_level(GPIO_NUM_15, 1);
                esp_rom_delay_us(150);
                gpio_set_level(GPIO_NUM_15, 0);
                esp_rom_delay_us(150);
            }
        }
        xSemaphoreGive(xMutex);
        vTaskDelay(pdMS_TO_TICKS(400)); // tiempo de ejecución para TareaLP
    }
}

void buttonTask(void *arg)
{
    gpio_set_direction(GPIO_NUM_13, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_13, GPIO_PULLUP_ONLY);

    while (1)
    {
        if (gpio_get_level(GPIO_NUM_13) == 0)
        {
            stopLED = true; // Detener el parpadeo si el botón está presionado
        }
        else
        {
            stopLED = false; // Continuar parpadeo si el botón está suelto
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Pequeña espera para evitar rebotes
    }
}

void app_main()
{
    xMutex = xSemaphoreCreateMutex(); // Se crea el semaforo y se asigna al objeto creado

    // Configuracion del GPIO15 y GPIO2 como OUTPUT en LOW
    esp_rom_gpio_pad_select_gpio(GPIO_NUM_15);
    gpio_set_direction(GPIO_NUM_15, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_15, 0);
    esp_rom_gpio_pad_select_gpio(GPIO_NUM_2);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_2, 0);

    xTaskCreatePinnedToCore(tareaLP, "tareaLP", 4096, NULL, 1, NULL, 0); // Se crea una tarea de baja prioridad
    xTaskCreatePinnedToCore(tareaHP, "tareaHP", 4096, NULL, 4, NULL, 0); // Se crea una tarea de alta prioridad
    xTaskCreatePinnedToCore(buttonTask, "buttonTask", 2048, NULL, 2, NULL, 0); // Se crea una tarea para el botón
}