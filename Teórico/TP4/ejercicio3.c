#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char** argv) {
    // Array de argumentos. El primer elemento siempre es el nombre del programa
    char *args[] = {"./ejercicio3_exec", NULL};

    printf("Proceso Original a punto de ejecutarse con PID: %d\n", getpid());
    printf("Reemplazando imagen de proceso actual por './ejercicio3_exec'...\n----------------------\n");

    // execv busca el ejecutable en la ruta explícita (a diferencia de execvp que usa el PATH)
    execv("./ejercicio3_exec", args);

    // Si execv tiene éxito, JAMÁS se ejecutan las líneas de abajo, la imagen anterior de RAM se aniquila.
    printf("execv() failed. Error: %s\n", strerror(errno));

    return 0;
}