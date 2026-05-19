#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <PID>\n", argv[0]);
        return 1;
    }
    
    pid_t pid = atoi(argv[1]);
    if (kill(pid, SIGINT) == -1) {
        perror("Error al enviar la señal");
        return 1;
    }
    
    printf("Señal SIGINT enviada exitosamente al proceso %d\n", pid);
    return 0;
}
