#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/control_led.sock"

int main(int argc, char *argv[]) {
    // 1. Validar la entrada por consola
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <ON|OFF|STATUS>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *comando = argv[1];

    // Validación básica de los comandos permitidos en cliente
    if (strcmp(comando, "ON") != 0 && strcmp(comando, "OFF") != 0 && strcmp(comando, "STATUS") != 0) {
        fprintf(stderr, "Comando invalido. Use ON, OFF o STATUS.\n");
        exit(EXIT_FAILURE);
    }

    // 2. Crear el socket Unix
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // 3. Configurar la dirección objetivo del servidor
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // 4. Conectar al servidor (que debe estar en ejecución)
    if (connect(sock, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Error al conectar con el Servidor Daemon. ¿Esta corriendo?");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 5. Enviar el comando al servidor
    if (write(sock, comando, strlen(comando)) == -1) {
        perror("Error al enviar el comando");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 6. Esperar y recibir la confirmación
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));

    ssize_t bytes_leidos = read(sock, buffer, sizeof(buffer) - 1);
    if (bytes_leidos > 0) {
        printf("Respuesta del Servidor: %s\n", buffer);
    } else {
        printf("El servidor cerro la conexion sin responder.\n");
    }

    // 7. Cerrar conexión
    close(sock);
    return 0;
}