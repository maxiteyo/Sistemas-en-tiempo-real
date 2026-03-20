#include <stdio.h>
#include <unistd.h> // Necesario para la funcion sleep() en Linux

int main() {
    int ciclos_totales = 3;

    printf("--- Iniciando Simulador de Semaforo ---\n\n");

    // Bucle para repetir el ciclo del semáforo varias veces
    for (int i = 0; i < ciclos_totales; i++) {
        
        // --- ESTADO VERDE ---
        printf("Semaforo en VERDE (Avanzar) - 5 segundos\n");
        sleep(5); // Pausa el programa por 5 segundos
        
        // --- ESTADO AMARILLO ---
        printf("Semaforo en AMARILLO (Precaucion) - 2 segundos\n");
        sleep(2); // Pausa el programa por 2 segundos
        
        // --- ESTADO ROJO ---
        printf("Semaforo en ROJO (Detenerse) - 4 segundos\n");
        sleep(4); // Pausa el programa por 4 segundos
        
        // Separador para que se lea mejor en la consola
        printf("-----------------------------\n");
    }

    printf("\nSimulacion terminada.\n");

    return 0;
}
