#include <stdlib.h>
#include <stdio.h>

int main() {
    float f;
    printf("Ingrese un numero flotante: ");
    scanf("%f", &f);
    short s = (short)f;
    printf("El numero flotante %f convertido a short es: %d\n", f, s);
    return 0;
}