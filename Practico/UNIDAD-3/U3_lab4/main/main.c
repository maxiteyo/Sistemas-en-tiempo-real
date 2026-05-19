#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_random.h"

// Parámetros de la cola
#define QUEUE_LENGTH 10
#define ITEM_SIZE sizeof(int)

// Handle de la cola
QueueHandle_t sensor_queue;
static const char *TAG = "INTER-CORE";

// A. Tarea Productora (Core 1)
void producer_task(void *pvParameters) {
    int sensor_value;
    while (1) {
        // Simular lectura: número aleatorio entre 0 y 100
        sensor_value = esp_random() % 101;

        // Enviar a la cola. Espera hasta 10ms si está llena (10ms a Ticks)
        if (xQueueSend(sensor_queue, &sensor_value, pdMS_TO_TICKS(10)) == pdTRUE) {
            UBaseType_t msgs_waiting = uxQueueMessagesWaiting(sensor_queue);
            ESP_LOGI(TAG, "Productor (Core %d): Genero y envio el valor -> %d | Ocupacion de cola: %d/%d", 
                     xPortGetCoreID(), sensor_value, (int)msgs_waiting, QUEUE_LENGTH);
        } else {
            ESP_LOGW(TAG, "Productor (Core %d): Cola llena, dato descartado.", xPortGetCoreID());
        }

        // Frecuencia: Ejecución cada 500ms
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// B. Tarea de Procesamiento Consumidora (Core 0)
void consumer_task(void *pvParameters) {
    int received_value;
    while (1) {
        // Permanecer en estado Blocked (portMAX_DELAY) hasta que haya datos
        if (xQueueReceive(sensor_queue, &received_value, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Consumidor (Core %d): Valor recibido: %d", xPortGetCoreID(), received_value);
            ESP_LOGI(TAG, "Consumidor (Core %d): Procesando...", xPortGetCoreID());
            
            // Retardo para "simular carga de trabajo" pesada.
            // Al ser 1500ms (más lento que los 500ms del productor), forzamos a que la cola se vaya llenando.
            vTaskDelay(pdMS_TO_TICKS(1500));
        }
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Inicio de sistema. Creando Cola...");

    // 1. Instanciación de la cola antes del lanzamiento de tareas
    sensor_queue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);
    
    if (sensor_queue == NULL) {
        ESP_LOGE(TAG, "Error: No se pudo crear la cola de FreeRTOS");
        return;
    }

    ESP_LOGI(TAG, "Instanciando Tareas...");

    // 3. Aislamiento
    // Tare Productora: Core 1, Prioridad 2 (Media)
    xTaskCreatePinnedToCore(
        producer_task,     // Función de la tarea
        "Producer",        // Nombre
        2048,              // Tamaño de Stack
        NULL,              // Parámetros
        2,                 // Prioridad
        NULL,              // Handle (no requerido aquí)
        1                  // Anclado al Core 1
    );

    // Tarea Consumidora: Core 0, Prioridad 1 (Baja)
    xTaskCreatePinnedToCore(
        consumer_task,     // Función de la tarea
        "Consumer",        // Nombre
        2048,              // Tamaño de Stack
        NULL,              // Parámetros
        1,                 // Prioridad
        NULL,              // Handle
        0                  // Anclado al Core 0
    );
}
