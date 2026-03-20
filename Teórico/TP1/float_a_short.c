#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

int main() {
    float f;
    printf("Ingrese un numero flotante: ");
    scanf("%f", &f);
    
    signed short s;
    
    // Aplicar "Saturacion aritmetica"
    if (f > SHRT_MAX) {
        s = SHRT_MAX;
        printf("Saturacion: El numero ingreso supero el tope maximo.\n");
    } else if (f < SHRT_MIN) {
        s = SHRT_MIN;
        printf("Saturacion: El numero ingreso es menor al limite minimo.\n");
    } else {
        s = (signed short)f;
    }
    
    printf("El numero de punto flotante %f convertido a short es: %d\n", f, s);
    
    return 0;
}