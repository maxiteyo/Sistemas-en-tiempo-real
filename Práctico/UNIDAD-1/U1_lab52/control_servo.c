#include <stdio.h>
#include <stdlib.h>
#include <pigpio.h>
#include <unistd.h>

#define SERVO_PIN 18
#define BUTTON_PIN 17
#define DEBOUNCE_TIME_US 200000 // 200 ms convertidos a microsegundos

// Variables globales volátiles porque van a ser modificadas por una ISR
volatile int current_angle = 0;
volatile uint32_t last_tick = 0;

// Rutina de Servicio de Interrupción (ISR)
void button_isr(int gpio, int level, uint32_t tick) {
    // DIAGNÓSTICO CRUDO: Imprimir cualquier cosa que pase en el pinlast_tick
    printf("[Debug] Interrupcion disparada. Nivel leído: %d\n", level);

    // Filtramos para que solo reaccione al Flanco de Subida (cuando level es 1)
    if (level == 1) {
        // Calculamos el tiempo desde la última pulsación para el debounce
        if (tick - last_tick > DEBOUNCE_TIME_US) {
            last_tick = tick; // Actualizamos el último tiempo válido

            printf("\n=> [AVISO] ¡Botón físico pulsado (Flanco de subida detectado)!\n");

            // Lógica de estado (Máquina de estados finitos)
            current_angle += 30; // Incrementamos 30 grados
            
            // Si superamos o llegamos a 180, reseteamos a 0last_tick
            if (current_angle > 180) {
                current_angle = 0;
            }

            // Interpolación para mapear grados (0-180) a microsegundos (500-2500)
            // 0 grados = 500 us, 180 grados = 2500 us
            int pulse_width = 500 + (current_angle * 2000) / 180;

            // Enviamos el pulso de hardware al Servomotor
            gpioServo(SERVO_PIN, pulse_width);

            // Imprimimos información por consola como pide la consigna
            printf("Evento ISR: Servo movido a %d grados. (Pulso: %d us)\n", current_angle, pulse_width);
        }
    }
}

int main() {
    // 1. Inicializamos la librería pigpio
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Fallo al inicializar pigpio. Asegurate de correr como root (sudo)\n");
        return 1;
    }

    // 2. Configuración del Pin del Botón (Entrada y Pull-Down)
    gpioSetMode(BUTTON_PIN, PI_INPUT);
    //gpioSetPullUpDown(BUTTON_PIN, PI_PUD_DOWN);

    // 3. Estado inicial: movemos el servo a 0°
    gpioServo(SERVO_PIN, 500); 
    printf("Sistema iniciado - Esperando eventos. Servo en 0 grados (500us)\n");

    // 4. Configurar la Interrupción
    // Usamos gpioSetAlertFunc en lugar de gpioSetISRFunc ya que en las versiones 
    // modernas de Raspberry Pi OS es mucho más estable y funciona directo por DMA.
    gpioSetAlertFunc(BUTTON_PIN, button_isr);

    // 5. Mantenemos el programa principal vivo para que las interrupciones
    // puedan funcionar asincrónicamente en segundo plano.
    while (1) {
        sleep(1); // El CPU descansa profundamente y procesa otras cosas
    }

    // 6. Limpieza antes de salir
    gpioTerminate();
    return 0;
}
