#include <stdio.h>
#include <unistd.h>

int main() {
    printf("[Ejecutable 1] ¡Ejecutado con exito! Soy una imagen nueva. PID: %d\n", getpid());
    return 0;
}