#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_timer.h" // Para medir tiempos en microsegundos con alta precisión

// =========================================================================
// 1. CONFIGURACIÓN Y CONSTANTES DEL HARDWARE
// =========================================================================
#define LED_PIN     GPIO_NUM_2  // LED integrado en la placa NodeMCU-32S
#define BUTTON_PIN  GPIO_NUM_0  // Pin para el pulsador

// =========================================================================
// 2. VARIABLES GLOBALES (Semáforos y Estado)
// =========================================================================
SemaphoreHandle_t semaforo_isr; // Semáforo Binario
volatile int64_t tiempo_ultimo_evento = 0;
int estado_led = 0;

// =========================================================================
// 3. RUTINA DE INTERRUPCIÓN (ISR - Hardware)
// =========================================================================
void IRAM_ATTR isr_pulsador(void* arg) {
    // Variable requerida por FreeRTOS para saber si despertar una tarea prioritaria
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // Entrega el semáforo desde contexto de interrupción (No bloqueante)
    xSemaphoreGiveFromISR(semaforo_isr, &xHigherPriorityTaskWoken);
    
    // Fuerza un cambio de contexto rápido si despertamos a la Tarea de Procesamiento
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

// =========================================================================
// 4. TAREA A: PROCESAMIENTO (Consumidor - Core 1 - Alta Prioridad: 3)
// =========================================================================
void tarea_procesamiento(void *pvParameters) {
    int64_t tiempo_actual;
    int64_t tiempo_transcurrido;

    while (1) {
        // La tarea queda bloqueada (consumo de CPU = 0%) esperando el Semáforo Binario
        if (xSemaphoreTake(semaforo_isr, portMAX_DELAY) == pdTRUE) {
            
            // Calculamos el tiempo transcurrido desde el pulsador anterior (en milisegundos)
            tiempo_actual = esp_timer_get_time(); 
            tiempo_transcurrido = (tiempo_actual - tiempo_ultimo_evento) / 1000;
            
            // FILTRO ANTI-REBOTES (Ignoramos interrupciones que ocurran dentro de 50ms)
            if (tiempo_transcurrido > 50) {
                tiempo_ultimo_evento = tiempo_actual;

                // Acción: Conmutar el LED
                estado_led = !estado_led;
                gpio_set_level(LED_PIN, estado_led);

                // Requerimiento: Imprimir tiempo transcurrido
                printf("[Procesamiento] Interrupcion capturada! LED: %s | Tiempo desde ultimo evento: %lld ms\n", 
                       estado_led ? "ON" : "OFF", 
                       tiempo_transcurrido);
            }
        }
    }
}

// =========================================================================
// 5. TAREA B: TELEMETRÍA (Core 1 - Baja Prioridad: 1)
// =========================================================================
void tarea_telemetria(void *pvParameters) {
    while (1) {
        printf("[Telemetria] Sistema Operativo Saludable\n");
        // Dormir esta tarea exactamente 5 segundos (5000 ms)
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// =========================================================================
// 6. MAIN (Orquestación en ESP-IDF)
// =========================================================================
void app_main() {
    printf("Iniciando Sistema RTOS Productor-Consumidor...\n");

    // A. Crear Semáforo Binario
    semaforo_isr = xSemaphoreCreateBinary();
    if (semaforo_isr == NULL) {
        printf("Error fatal: No se pudo crear el semaforo.\n");
        return;
    }

    // B. Configurar Pin del LED
    gpio_reset_pin(LED_PIN);
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    // C. Configurar Pin del Pulsador con Pull-Up interno
    gpio_config_t btn_config = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,    // Activa la resistencia interna hacia 3.3V
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE      // Evento: Flanco de Bajada
    };
    gpio_config(&btn_config);

    // Instalar el servicio de interrupciones globales y conectar la ISR al pin 4
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, isr_pulsador, NULL);

    tiempo_ultimo_evento = esp_timer_get_time(); // Seteamos el tiempo cero inicial

    // D. Crear Tareas y asignarlas a los Núcleos correspondientes
    // Tarea de Procesamiento en Core 1 (Prioridad 3)
    xTaskCreatePinnedToCore(
        tarea_procesamiento,       // Función de la tarea
        "Procesamiento",           // Nombre (Para debug)
        2048,                      // Tamaño de pila (Stack RAM)
        NULL,                      // Parámetros de la tarea
        3,                         // Nivel de Prioridad (Alta)
        NULL,                      // Puntero o Handle de la tarea
        1                          // Fijado al Core 1
    );

    // Tarea de Telemetría en Core 1 (Prioridad 1)
    xTaskCreatePinnedToCore(
        tarea_telemetria,
        "Telemetria",
        2048,
        NULL,
        1,                         // Nivel de Prioridad (Baja)
        NULL,
        1                          // Fijado al Core 1
    );

    printf("Setup finalizado. ESP32 ejecutando FreeRTOS...\n");
}