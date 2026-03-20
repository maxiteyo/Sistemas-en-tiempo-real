#include <stdio.h>
#include <pigpio.h> // Librería para manipular pines
#include <unistd.h> // Para la función sleep() principal

#define LED_PIN 18      // Pin donde conectamos el LED (BCM 18)
#define BOTON_PIN 17    // Pin donde conectamos el botón (BCM 17)

// 1. Definimos la Rutina de Servicio de Interrupción (ISR / Callback)
// Esta función no la llamamos nosotros manualmente. La llama el hardware de la
// Raspberry de forma automática cuando detecta que el voltaje del botón cambió.
void interrupcion_boton(int gpio, int nivel, uint32_t tick) {
    // nivel == 0 significa que el voltaje cayó a Tierra (GND), o sea, ALGUIEN PRESIONÓ
    if (nivel == 0) { 
        gpioWrite(LED_PIN, 1);      // Encendemos el LED
        printf("BOTÓN_PRESIONADO\n");
    } 
    // nivel == 1 significa que el voltaje subió a 3.3V, o sea, ALGUIEN SOLTÓ
    else if (nivel == 1) { 
        gpioWrite(LED_PIN, 0);      // Apagamos el LED
        printf("BOTÓN_LIBERADO\n");
    }
}

int main() {
    // 2. Inicializar pigpio
    if (gpioInitialise() < 0) {
        printf("Error: Fallo al inicializar pigpio. Usa sudo.\n");
        return -1;
    }

    // 3. Configurar el LED como Salida
    gpioSetMode(LED_PIN, PI_OUTPUT);

    // 4. Configurar el Botón como Entrada
    gpioSetMode(BOTON_PIN, PI_INPUT);
    
    // 5. Configurar Resistencia Interna (Pull-Up)
    // Esto asegura que si el botón no está apretado, el pin lea un 1 estable (3.3V)
    // El botón físico debe estar conectado entre el pin GPIO 17 y un pin GND (Tierra).
    gpioSetPullUpDown(BOTON_PIN, PI_PUD_UP);

    // 6. Filtro Anti-Rebote (Glitch Filter)
    // Los botones mecánicos "rebotan" microscópicamente y envían muchos ceros y unos 
    // de golpe al tacto. Esto filtra los ruidos de menos de 30,000 microsegundos (30ms).
    gpioGlitchFilter(BOTON_PIN, 30000);

    // 7. Vincular la función a la interrupción de hardware ("Alert Func")
    // Le decimos a pigpio: "Vigila el BOTON_PIN. Cuando cambie, ejecuta interrupcion_boton"
    gpioSetAlertFunc(BOTON_PIN, interrupcion_boton);

    printf("--- Programa de Boton con Interrupciones Iniciado ---\n");
    printf("Esperando evento en hardware... Presiona Ctrl+C para salir.\n");

    // 8. Bucle Infinito del programa principal
    // A diferencia de un sistema sin interrupciones que tendría que estar preguntando
    // "Boton apretado? Boton apretado? Boton apretado?" gastando CPU, aquí la CPU descansa.
    while (1) {
        sleep(1); // El programa duerme tranquilamente, la interrupción lo despertará si hace falta
    }

    // Limpieza
    gpioTerminate();
    return 0;
}
