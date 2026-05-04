#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

#define NUM_HILOS 10

// Función complementaria: Una sola función para los 10 hilos
void* funcion_hilo(void* arg) {
    // Obtenemos el número lógico que le pasamos desde el main
    int id_logico = *((int*)arg); 
    
    // Obtenemos el TID real asignado por el Kernel de Linux
    pid_t tid = syscall(SYS_gettid);
    
    // Obtenemos el ID de POSIX
    pthread_t ptid = pthread_self();

    printf("Hilo [%02d] en ejecución -> TID Kernel: %d | TID POSIX: %lx\n", 
           id_logico, tid, (unsigned long)ptid);

    // Liberamos la memoria solicitada en el main para este argumento
    free(arg); 
    
    return NULL;
}

int main() {
    // Arreglo para almacenar los identificadores de los 10 hilos
    pthread_t arreglo_hilos[NUM_HILOS];

    printf("Iniciando orquestador. Creando arreglo de %d hilos...\n\n", NUM_HILOS);

    // Bucle generador de hilos
    for (int i = 0; i < NUM_HILOS; i++) {
        // Alojamos memoria dinámica para pasarle el número de hilo (1 al 10)
        // Esto evita "condiciones de carrera" al leer la variable 'i'
        int* id_arg = malloc(sizeof(int));
        *id_arg = i + 1; 

        // Creación del hilo guardando su ID en el arreglo
        if (pthread_create(&arreglo_hilos[i], NULL, funcion_hilo, id_arg) != 0) {
            perror("Error al crear un hilo");
            return 1;
        }
    }

    // Bucle recolector (Join) para evitar que el main muera antes
    for (int i = 0; i < NUM_HILOS; i++) {
        if (pthread_join(arreglo_hilos[i], NULL) != 0) {
            perror("Error al esperar al hilo");
            return 1;
        }
    }

    printf("\nTodos los hilos han reportado su TID y finalizado exitosamente.\n");
    return 0;
}
