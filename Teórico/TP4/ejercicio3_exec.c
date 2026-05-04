#include <stdio.h>
#include <unistd.h>

int main() {
    printf("¡Hola! Soy el NUEVO EJECUTABLE LOCAL que se cargo en memoria.\n");
    printf("Mi PID es: %d\n", getpid());
    return 0;
}