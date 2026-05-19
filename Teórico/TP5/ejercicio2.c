#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sched.h>
#include <time.h>

#define NUM_CHILDREN 3

void print_process_info(int id) {
    int pol;
    struct sched_param sp;
    pid_t pid = getpid();

    pol = sched_getscheduler(0);
    if (pol == -1) perror("sched_getscheduler");
    
    if (sched_getparam(0, &sp) == -1) perror("sched_getparam");

    int nice_val = getpriority(PRIO_PROCESS, 0);

    const char *pol_str = "UNKNOWN";
    switch(pol){
        case SCHED_OTHER: pol_str = "SCHED_OTHER"; break;
        case SCHED_FIFO:  pol_str = "SCHED_FIFO"; break;
        case SCHED_RR:    pol_str = "SCHED_RR"; break;
    }

    printf("[Hijo %d - PID %d] Pol: %s | Prio: %d | Nice: %d\n", id, pid, pol_str, sp.sched_priority, nice_val);
}

void child_task(int id) {
    for (int i = 0; i < 15; i++) {
        print_process_info(id);
        sleep(1);
    }
    printf("[Hijo %d - PID %d] Terminando.\n", id, getpid());
    exit(0);
}

int main() {
    pid_t children[NUM_CHILDREN];

    printf("[Padre - PID %d] Iniciando creación de hijos...\n", getpid());

    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Código del hijo
            child_task(i + 1);
        } else {
            // Código del padre
            children[i] = pid;
        }
    }

    printf("[Padre] Hijos creados. Esperando 10 segundos para cambiar prioridades...\n");
    sleep(10);

    printf("\n[Padre] Cambiando prioridades de los hijos...\n");
    for (int i = 0; i < NUM_CHILDREN; i++) {
        // En Linux por defecto se usa SCHED_OTHER, por lo que modificamos el valor nice.
        // Aumentamos el valor nice (reducimos la prioridad) de forma distinta para cada hijo.
        int nuevo_nice = 5 + (i * 5); // nice: 5, 10, 15
        if (setpriority(PRIO_PROCESS, children[i], nuevo_nice) == -1) {
            perror("setpriority");
        } else {
            printf("[Padre] Hijo %d (PID %d) - Nuevo valor nice: %d\n", i + 1, children[i], nuevo_nice);
        }
    }

    // Esperar a que terminen los hijos
    for (int i = 0; i < NUM_CHILDREN; i++) {
        wait(NULL);
    }

    printf("[Padre] Todos los hijos han terminado.\n");
    return 0;
}
