#include <stdio.h>
#include <pigpio.h>
#include <unistd.h>
#include <stdint.h> 

#define LED_PIN 18
#define BOTON_PIN 17
#define TOTAL_MEDICIONES 20 // Cantidad mínima pedida en la consigna

// Variables Globales 
// Usamos "volatile" para advertirle al compilador que estas variables 
// van a ser modificadas por una interrupción en cualquier momento.
volatile int contador_mediciones = 0;
volatile uint32_t latencias[TOTAL_MEDICIONES]; // Arreglo para guardar los datos

// Función de Interrupción
void interrupcion_boton(int gpio, int nivel, uint32_t tick_evento) {
    // Si el botón es presionado (nivel 0)
    if (nivel == 0) { 
        // 1. Inmediatamente tomamos el tiempo en que entra a encender el LED (Tencendido_led)
        uint32_t tick_encendido = gpioTick(); 

        // 2. Encendemos el LED
        gpioWrite(LED_PIN, 1);
        
        // 3. Registramos la latencia si aún no llegamos a 20
        if (contador_mediciones < TOTAL_MEDICIONES) {
            // Latencia = Tencendido_led - Tevento_boton
            latencias[contador_mediciones] = tick_encendido - tick_evento;
            contador_mediciones++;
            printf("Medicion %d/%d completada.\n", contador_mediciones, TOTAL_MEDICIONES);
        }
    } 
    // Si el botón es liberado (nivel 1), solo apagamos. No medimos nada aquí.
    else if (nivel == 1) {
        gpioWrite(LED_PIN, 0);
    }
}

int main() {
    // Inicialización y configuración estándar
    if (gpioInitialise() < 0) return -1;

    gpioSetMode(LED_PIN, PI_OUTPUT);
    gpioSetMode(BOTON_PIN, PI_INPUT);
    //gpioSetPullUpDown(BOTON_PIN, PI_PUD_UP);
    gpioGlitchFilter(BOTON_PIN, 30000); // Filtro anti-rebote

    // Enganchamos la interrupción
    gpioSetAlertFunc(BOTON_PIN, interrupcion_boton);

    printf("--- Analisis de Latencia y Jitter ---\n");
    printf("Presiona el boton %d veces...\n\n", TOTAL_MEDICIONES);

    // Bucle de espera activa: El main se queda aquí dando vueltas
    // hasta que el contador llegue a las 20 pulsaciones.
    while (contador_mediciones < TOTAL_MEDICIONES) {
        sleep(1); 
    }

    // --- FASE DE ANÁLISIS ESTADÍSTICO ---
    printf("\n--- Mediciones Recopiladas ---\n");
    
    uint32_t min = latencias[0];
    uint32_t max = latencias[0];
    uint64_t suma_total = 0; // Se usa un número de 64 bits para que no se desborde al sumar

    // Tabular y buscar min, max y suma iterando el arreglo
    for (int i = 0; i < TOTAL_MEDICIONES; i++) {
        printf("Pulsacion %2d: %u microsegundos\n", i + 1, latencias[i]);
        
        if (latencias[i] < min) min = latencias[i];
        if (latencias[i] > max) max = latencias[i];
        suma_total += latencias[i];
    }

    // Calcular promedios y jitter
    float promedio = (float)suma_total / TOTAL_MEDICIONES;
    uint32_t jitter = max - min;

    printf("\n--- Resultados Estadisticos ---\n");
    printf("Latencia Minima:   %u us\n", min);
    printf("Latencia Maxima:   %u us\n", max);
    printf("Latencia Promedio: %.2f us\n", promedio);
    printf("JITTER (Max-Min):  %u us\n", jitter);
    printf("-------------------------------\n");
    printf("Finalizando programa. Analisis exitoso.\n");

    gpioTerminate();
    return 0;
}
