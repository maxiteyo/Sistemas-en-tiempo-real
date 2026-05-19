// gcc -o ejercicio3 ejercicio3_hilos.c -lpthread

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

// Mutex para evitar que las impresiones en consola (printf) de distintos hilos se superpongan
pthread_mutex_t console_mtx;

void* Task1(void* arg) {
    long elapsed = 0;
    while (1) {
        // Pausa de 100 ms (100,000 microsegundos)
        usleep(100 * 1000); 
        elapsed += 100;
        
        pthread_mutex_lock(&console_mtx);
        printf("[Hilo 1] - 100ms - Tiempo transcurrido aprox: %ld ms\n", elapsed);
        pthread_mutex_unlock(&console_mtx);
    }
    return NULL;
}

void* Task2(void* arg) {
    long elapsed = 0;
    while (1) {
        // Pausa de 300 ms (300,000 microsegundos)
        usleep(300 * 1000);
        elapsed += 300;
        
        pthread_mutex_lock(&console_mtx);
        printf("[Hilo 2] - 300ms - Tiempo transcurrido aprox: %ld ms\n", elapsed);
        pthread_mutex_unlock(&console_mtx);
    }
    return NULL;
}

void* Task3(void* arg) {
    long elapsed = 0;
    while (1) {
        // Pausa de 500 ms (500,000 microsegundos)
        usleep(500 * 1000);
        elapsed += 500;
        
        pthread_mutex_lock(&console_mtx);
        printf("[Hilo 3] - 500ms - Tiempo transcurrido aprox: %ld ms\n", elapsed);
        pthread_mutex_unlock(&console_mtx);
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2, thread3;

    pthread_mutex_init(&console_mtx, NULL);

    printf("Iniciando Sistema Multihilo...\n");

    // Creación de los hilos independientes para cada tarea
    if (pthread_create(&thread1, NULL, Task1, NULL) != 0 ||
        pthread_create(&thread2, NULL, Task2, NULL) != 0 ||
        pthread_create(&thread3, NULL, Task3, NULL) != 0) {
        
        perror("Error al crear los hilos");
        exit(EXIT_FAILURE);
    }

    // Esperar a que terminen los hilos (aunque los loops son infinitos y la terminación real
    // normalmente se detendría con Ctrl+C)
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    pthread_mutex_destroy(&console_mtx);

    return 0;
}