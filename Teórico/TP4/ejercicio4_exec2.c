#include <stdio.h>
#include <unistd.h>

int main() {
    printf("[Ejecutable 2] ¡Tambien ejecutado con exito! Soy otra imagen nueva. PID: %d\n", getpid());
    return 0;
}