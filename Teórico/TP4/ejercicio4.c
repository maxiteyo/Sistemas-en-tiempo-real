#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid1, pid2;

    printf("Proceso Padre (Supervisor) iniciado. PID: %d\n", getpid());

    // ============================================
    // PRIMER HIJO
    // ============================================
    pid1 = fork();
    if (pid1 == 0) {
        printf("-> Hijo 1 (PID: %d) creado. Reemplazando imagen con exec1...\n", getpid());
        char *args[] = {"./ejercicio4_exec1", NULL};
        
        // execv aniquila este proceso hijo y le carga el ejecutable 1
        execv("./ejercicio4_exec1", args);
        
        // Solo llega aquí si falla execv
        perror("Error execv hijo 1");
        exit(EXIT_FAILURE);
    }

    // ============================================
    // SEGUNDO HIJO
    // ============================================
    pid2 = fork();
    if (pid2 == 0) {
        printf("-> Hijo 2 (PID: %d) creado. Reemplazando imagen con exec2...\n", getpid());
        char *args[] = {"./ejercicio4_exec2", NULL};
        
        // execv aniquila este proceso hijo y le carga el ejecutable 2
        execv("./ejercicio4_exec2", args);
        
        // Solo llega aquí si falla execv
        perror("Error execv hijo 2");
        exit(EXIT_FAILURE);
    }

    // ============================================
    // PADRE ESPERA A AMBOS
    // ============================================
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    printf("Proceso Padre (PID: %d): Mis dos hijos han terminado su ejecucion.\n", getpid());
    return 0;
}