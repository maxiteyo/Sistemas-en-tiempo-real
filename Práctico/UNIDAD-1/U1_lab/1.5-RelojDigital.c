#include <stdio.h>
#include <time.h>    // Biblioteca para manejar fechas y horas
#include <unistd.h>  // Para usar sleep()

int main() {
    time_t tiempo_actual;
    struct tm *info_tiempo;
    char buffer_hora[50];

    printf("--- Reloj Digital ---\n");
    printf("Presiona Ctrl+C en la terminal para salir.\n\n");

    // Bucle infinito: el reloj corre hasta que el usuario lo cancele
    while (1) {
        // 1. Obtener la cantidad de segundos desde el 1 de enero de 1970 (Epoch de UNIX)
        tiempo_actual = time(NULL);

        // 2. Convertir esos segundos al formato de hora local de tu computadora
        info_tiempo = localtime(&tiempo_actual);

        // 3. Formatear la hora en un texto legible (Hora:Minutos:Segundos)
        strftime(buffer_hora, sizeof(buffer_hora), "%H:%M:%S", info_tiempo);

        // 4. Mostrar la hora en consola. 
        // El "\r" (retorno de carro) hace que el cursor vuelva al inicio de la misma linea.
        printf("\rHora actual: %s", buffer_hora);

        // 5. Forzar a la terminal a imprimir el texto inmediatamente
        fflush(stdout);

        // 6. Esperar 1 segundo antes de la siguiente actualización
        sleep(1);
    }

    return 0;
}
