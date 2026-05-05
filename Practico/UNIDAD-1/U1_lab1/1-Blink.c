#include <stdio.h>
#include <stdbool.h>
#include <pigpio.h> // Librería obligatoria para manipular los pines de la Raspberry

// Definimos el pin GPIO donde está conectado el LED. 
// ATENCIÓN: Usar numeración BCM (Broadcom), NO el número de pata física.
// Usaremos el GPIO 18 como ejemplo (pin físico 12).
#define LED_PIN 18  

// Requerimiento: 500ms. pigpio mide el tiempo en microsegundos (µs).
// 500 milisegundos = 500,000 microsegundos.
#define INTERVALO_US 500000 

int main() {
    // 1. Inicialización de la librería pigpio
    if (gpioInitialise() < 0) {
        printf("Error: Fallo al inicializar la libreria pigpio. ¿Ejecutaste el programa con sudo?\n");
        return -1;
    }

    // 2. Configurar el PIN del LED como SALIDA
    gpioSetMode(LED_PIN, PI_OUTPUT);

    // 3. Variables para el Timer "No Bloqueante"
    // gpioTick() devuelve la cantidad de microsegundos desde que encendió la Raspberry.
    uint32_t tiempo_anterior = gpioTick(); 
    uint32_t tiempo_actual;
    bool estado_led = false; // Empezamos asumiendo que el LED está apagado

    printf("Iniciando parpadeo del LED (Timer No Bloqueante)...\n");
    printf("Presiona Ctrl+C para salir.\n");

    // 4. Bucle infinito principal (Super Loop)
    while (1) {
        // Tomamos una lectura actual del "reloj"
        tiempo_actual = gpioTick(); 

        // Verificamos si ya pasaron 500.000 microsegundos
        if (tiempo_actual - tiempo_anterior >= INTERVALO_US) {
            
            // Cambiamos el estado (si era false(0) pasa a true(1), y viceversa)
            estado_led = !estado_led; 

            // Escribimos en el pin físico de la Raspberry
            // El estado_led devuelve 1 o 0 gracias a la librería stdbool
            gpioWrite(LED_PIN, estado_led ? 1 : 0);

            // Imprimimos en pantalla conforme a la consigna
            if (estado_led) {
                printf("LED_ON\n");
            } else {
                printf("LED_OFF\n");
            }

            // Actualizamos la marca de tiempo para el siguiente ciclo.
            // Le sumamos el intervalo al tiempo anterior para una mayor precisión.
            tiempo_anterior += INTERVALO_US; 
        }

        // (Opcional pero muy recomendado) 
        // Agregamos un mínimo delay de 1 milisegundo (1000 µs) para que el bucle
        // no sature el procesador al 100% mientras verifica la condición anterior.
        // Esto NO rompe la consigna, ya que el programa "respira" pero no se bloquea ciegamente
        gpioDelay(1000);
    }

    // Código final de limpieza (rara vez se llega aquí si no detenemos el bucle explícitamente)
    gpioTerminate();
    return 0;
}
