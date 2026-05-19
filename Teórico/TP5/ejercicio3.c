#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sched.h> // Para SCHED_OTHER, etc.
#include <sys/syscall.h>

#define NUM_THREADS 3

typedef struct {
    int id;
    pthread_t thread_id;
    pid_t tid; // Identificador del hilo a nivel del OS para setpriority
} thread_info_t;

thread_info_t tinfo[NUM_THREADS];

void* thread_task(void *arg) {
    thread_info_t *info = (thread_info_t *)arg;
    info->tid = syscall(SYS_gettid); // Guardar TID para poder cambiar su nice value desde el main

    for (int i = 0; i < 15; i++) {
        int pol;
        struct sched_param sp;

        // Obtener la política y prioridad POSIX del hilo
        if (pthread_getschedparam(pthread_self(), &pol, &sp) != 0) {
            perror("pthread_getschedparam");
        }

        // En Linux, cada hilo tiene su propio valor nice.
        int nice_val = getpriority(PRIO_PROCESS, 0);

        const char *pol_str = "UNKNOWN";
        switch(pol){
            case SCHED_OTHER: pol_str = "SCHED_OTHER"; break;
            case SCHED_FIFO:  pol_str = "SCHED_FIFO"; break;
            case SCHED_RR:    pol_str = "SCHED_RR"; break;
        }

        printf("[Hilo %d - TID %d] Pol: %s | Prio POSIX: %d | Nice: %d\n", 
               info->id, info->tid, pol_str, sp.sched_priority, nice_val);

        sleep(1);
    }

    printf("[Hilo %d - TID %d] Terminando.\n", info->id, info->tid);
    return NULL;
}

int main() {
    printf("[Principal - PID %d] Iniciando creación de hilos...\n", getpid());

    for (int i = 0; i < NUM_THREADS; i++) {
        tinfo[i].id = i + 1;
        if (pthread_create(&tinfo[i].thread_id, NULL, thread_task, &tinfo[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Esperamos un segundo para asegurarnos de que los hilos guardaron su TID
    sleep(1);

    printf("[Principal] Hilos creados. Esperando 9 segundos para cambiar prioridades...\n");
    sleep(9);

    printf("\n[Principal] Cambiando prioridades (valores nice) de los hilos...\n");
    for (int i = 0; i < NUM_THREADS; i++) {
        int nuevo_nice = 5 + (i * 5); // nice: 5, 10, 15
        
        // En Linux, usando PRIO_PROCESS pasandole el TID, cambia el nice del hilo en particular.
        if (setpriority(PRIO_PROCESS, tinfo[i].tid, nuevo_nice) == -1) {
            perror("setpriority");
        } else {
            printf("[Principal] Hilo %d (TID %d) - Nuevo valor nice configurado: %d\n", 
                    tinfo[i].id, tinfo[i].tid, nuevo_nice);
        }
    }

    // Esperar a que terminen los hilos
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(tinfo[i].thread_id, NULL);
    }

    printf("[Principal] Todos los hilos han terminado.\n");
    return 0;
}
