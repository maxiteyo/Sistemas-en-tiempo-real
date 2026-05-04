#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    printf("Iniciando Proceso Padre con PID: %d\n", getpid());

    for (int i = 0; i < 10; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            perror("Error en fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Código ejecutado SOLO por el proceso hijo
            printf("Soy el Hijo %d -> PID: %d, PPID (Padre): %d\n", i + 1, getpid(), getppid());
            
            // Pausamos el hijo 15 segundos para darte tiempo a correr "pstree" y "top" en otra terminal
            sleep(15); 
            
            exit(EXIT_SUCCESS); // El hijo muere acá, no sigue el bucle for
        }
    }
    
    // Código ejecutado SOLO por el padre: espera a que mueran los 10 hijos
    for (int i = 0; i < 10; i++) {
        wait(NULL);
    }
    
    printf("Todos los hijos terminaron. Proceso padre (PID: %d) finalizando.\n", getpid());
    return 0;
}