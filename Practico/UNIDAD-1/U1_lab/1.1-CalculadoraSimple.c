#include <stdio.h>

int main() {
    char operador;
    double num1, num2;

    // Solicitar al usuario el operador
    printf("Ingresa un operador (+, -, *, /): ");
    scanf(" %c", &operador); // El espacio antes de %c ignora espacios en blanco o saltos de línea previos

    // Solicitar al usuario los dos números
    printf("Ingresa el primer numero: ");
    scanf("%lf", &num1);
    
    printf("Ingresa el segundo numero: ");
    scanf("%lf", &num2);

    // Seleccionar la operación matemática según el operador ingresado
    switch (operador) {
        case '+':
            printf("%.2lf + %.2lf = %.2lf\n", num1, num2, num1 + num2);
            break;
        case '-':
            printf("%.2lf - %.2lf = %.2lf\n", num1, num2, num1 - num2);
            break;
        case '*':
            printf("%.2lf * %.2lf = %.2lf\n", num1, num2, num1 * num2);
            break;
        case '/':
            // Comprobación para evitar la división por cero
            if (num2 != 0.0) {
                printf("%.2lf / %.2lf = %.2lf\n", num1, num2, num1 / num2);
            } else {
                printf("Error: No se puede dividir por cero.\n");
            }
            break;
        default:
            printf("Error: Operador no valido.\n");
    }

    return 0;
}
