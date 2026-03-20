#include <stdio.h>

int main() {
    int n, opcion, temp;

    // 1. Solicitar la cantidad de números a ingresar
    printf("¿Cuantos numeros deseas ingresar? (Maximo 100): ");
    scanf("%d", &n);

    // Verificación de seguridad
    if (n <= 0 || n > 100) {
        printf("Cantidad no valida. Terminando programa.\n");
        return 1;
    }

    int numeros[100]; // Arreglo para almacenar los números

    // 2. Leer los números ingresados por el usuario
    for (int i = 0; i < n; i++) {
        printf("Ingresa el numero %d: ", i + 1);
        scanf("%d", &numeros[i]);
    }

    // 3. Preguntar el tipo de ordenamiento
    printf("\n¿Como deseas ordenarlos?\n");
    printf("1. Ascendente (Menor a Mayor)\n");
    printf("2. Descendente (Mayor a Menor)\n");
    printf("Elige una opcion (1 o 2): ");
    scanf("%d", &opcion);

    // 4. Algoritmo de ordenamiento (Bubble Sort / Método de la Burbuja)
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            
            // Condición para intercambiar según la opción elegida
            int intercambiar = 0;
            if (opcion == 1 && numeros[j] > numeros[j+1]) {
                intercambiar = 1; // Ascendente
            } else if (opcion == 2 && numeros[j] < numeros[j+1]) {
                intercambiar = 1; // Descendente
            }

            // Realizar el intercambio si se cumple la condición
            if (intercambiar) {
                temp = numeros[j];
                numeros[j] = numeros[j+1];
                numeros[j+1] = temp;
            }
        }
    }

    // 5. Mostrar el resultado
    printf("\nNumeros ordenados:\n");
    for (int i = 0; i < n; i++) {
        printf("%d ", numeros[i]);
    }
    printf("\n");

    return 0;
}
