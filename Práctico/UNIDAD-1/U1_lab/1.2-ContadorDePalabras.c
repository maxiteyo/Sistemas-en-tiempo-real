#include <stdio.h>
#include <ctype.h>

#define MAX_LONGITUD 1000

int main() {
    char texto[MAX_LONGITUD];
    int contador_palabras = 0;
    int en_palabra = 0;

    // Solicitar al usuario que ingrese una frase
    printf("Ingresa una cadena de texto: ");
    
    // Leer la cadena de texto, incluyendo espacios, hasta el salto de línea
    if (fgets(texto, sizeof(texto), stdin) != NULL) {
        
        // Recorrer cada carácter de la cadena
        for (int i = 0; texto[i] != '\0'; i++) {
            // Verificar si el carácter actual es un espacio en blanco, tabulador o salto de línea
            if (isspace((unsigned char)texto[i])) {
                en_palabra = 0; // Estamos fuera de una palabra
            } 
            // Si no es un espacio y no estábamos ya dentro de una palabra
            else if (en_palabra == 0) {
                en_palabra = 1; // Entramos a una nueva palabra
                contador_palabras++; // Incrementamos el contador
            }
        }
        
        // Imprimir el resultado
        printf("El numero de palabras ingresadas es: %d\n", contador_palabras);
    } else {
        printf("Error al intentar leer la entrada.\n");
    }

    return 0;
}
