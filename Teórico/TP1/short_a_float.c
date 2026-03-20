#include <stdlib.h>
#include <stdio.h>

int main(){

    signed short s;
    printf("Ingrese un numero entero de tipo short: ");
    scanf("%hd", &s);
    float f = (float)s;
    printf("El numero entero %hd convertido a float es: %f\n", s, f);

    return 0;
}