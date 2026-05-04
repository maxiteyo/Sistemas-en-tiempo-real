#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

// Función complementaria (el cuerpo del hilo)
void* funcion_hilo(void* arg) {
    // Obtener el PID (Process ID) del proceso que contiene a este hilo
    pid_t pid = getpid();
    
    // Obtener el TID (Thread ID) real a nivel del kernel de Linux
    pid_t tid = syscall(SYS_gettid);
    
    // Obtener el ID de POSIX (pthread_t) para comparación
    pthread_t ptid = pthread_self();

    printf("\n--- Ejecutando Función Complementaria (Hilo) ---\n");
    printf("PID del Proceso Padre : %d\n", pid);
    printf("TID del Kernel (OS)   : %d\n", tid);
    printf("TID de POSIX (Hex)    : %lx\n", (unsigned long)ptid);
    printf("------------------------------------------------\n\n");

    return NULL;
}

int main() {
    pthread_t mi_hilo;

    printf("Iniciando el programa principal...\n");
    printf("PID del hilo principal (main): %d\n", getpid());
    printf("TID del hilo principal (main): %ld\n", syscall(SYS_gettid));

    // 1. Crear el hilo llamando a la función complementaria
    if (pthread_create(&mi_hilo, NULL, funcion_hilo, NULL) != 0) {
        perror("Error al crear el hilo");
        return 1;
    }

    // 2. Esperar a que el hilo termine su ejecución para que el main no muera antes
    if (pthread_join(mi_hilo, NULL) != 0) {
        perror("Error al hacer join del hilo");
        return 1;
    }

    printf("El hilo ha finalizado con éxito. Terminando programa.\n");
    return 0;
}
