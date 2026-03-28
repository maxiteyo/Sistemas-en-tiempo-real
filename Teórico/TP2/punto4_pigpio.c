#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pigpio.h> // Librería para manejar GPIO en Raspberry Pi

// Definición de los pines para cada LED (puedes cambiarlos según tu conexión física)
#define LED_TAREA1 17
#define LED_TAREA2 27
#define LED_TAREA3 22

// Tics utilizando la función gpioTick de pigpio que devuelve microsegundos
// Lo envolveremos en una función para obtener milisegundos y asimilarlo al Ej1
unsigned long get_tics_ms() {
    return gpioTick() / 1000;
}

// Tarea 1: Período de 100 ms
void Task1(void) {
    static int state = 0;
    static unsigned long timer;
    static int led_state = 0;
    
    unsigned long Tics = get_tics_ms();

    switch (state) {
        case 0:
            gpioSetMode(LED_TAREA1, PI_OUTPUT); // Configurar pin como salida
            timer = Tics + 100; 
            state = 1;
            break;
        case 1:
            if (Tics >= timer) {
                led_state = !led_state; // Invertir estado
                gpioWrite(LED_TAREA1, led_state); // Encender o Apagar LED
                
                printf("[Tarea 1 - LED %d] - %s - Tics: %lu ms\n", LED_TAREA1, led_state ? "ON " : "OFF", Tics);
                timer = Tics + 100;
            }
            break;
    }
}

// Tarea 2: Período de 300 ms
void Task2(void) {
    static int state = 0;
    static unsigned long timer;
    static int led_state = 0;
    
    unsigned long Tics = get_tics_ms();

    switch (state) {
        case 0:
            gpioSetMode(LED_TAREA2, PI_OUTPUT);
            timer = Tics + 300;
            state = 1;
            break;
        case 1:
            if (Tics >= timer) {
                led_state = !led_state;
                gpioWrite(LED_TAREA2, led_state);
                
                printf("[Tarea 2 - LED %d] - %s - Tics: %lu ms\n", LED_TAREA2, led_state ? "ON " : "OFF", Tics);
                timer = Tics + 300;
            }
            break;
    }
}

// Tarea 3: Período de 500 ms
void Task3(void) {
    static int state = 0;
    static unsigned long timer;
    static int led_state = 0;
    
    unsigned long Tics = get_tics_ms();

    switch (state) {
        case 0:
            gpioSetMode(LED_TAREA3, PI_OUTPUT);
            timer = Tics + 500;
            state = 1;
            break;
        case 1:
            if (Tics >= timer) {
                led_state = !led_state;
                gpioWrite(LED_TAREA3, led_state);
                
                printf("[Tarea 3 - LED %d] - %s - Tics: %lu ms\n", LED_TAREA3, led_state ? "ON " : "OFF", Tics);
                timer = Tics + 500;
            }
            break;
    }
}

int main() {
    printf("Iniciando Ejecutivo Cíclico en Raspberry Pi...\n");

    // Inicializamos PIGPIO
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Error al inicializar pigpio. Asegúrate de ejecutarlo con 'sudo'.\n");
        return 1;
    }

    // Súper-bucle infinito del ejecutivo cíclico
    while (1) {
        Task1();
        Task2();
        Task3();
        
        // Pausa ligera para no ocupar el 100% del procesador
        time_sleep(0.001); // duerme 1 ms 
    }

    // Limpieza al salir (si tuviéramos manera de salir elegante)
    gpioTerminate();
    return 0;
}