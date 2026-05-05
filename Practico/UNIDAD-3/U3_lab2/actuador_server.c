#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <pigpio.h>

#define SOCKET_PATH "/tmp/control_led.sock"
#define LED_PIN 18  // Pin GPIO de ejemplo para el LED

// =========================================================================
// ESTRUCTURAS Y VARIABLES GLOBALES (Gestión de Estado)
// =========================================================================
int led_estado = 0;  // 0 = OFF, 1 = ON
int modo_simulado = 0; // Para correr sin Raspberry Pi si es necesario

// Mutex para proteger el estado del LED contra colisiones de múltiples clientes
pthread_mutex_t mutex_led = PTHREAD_MUTEX_INITIALIZER;

// =========================================================================
// HILOS WORKER: Atención al Cliente
// =========================================================================
void* atender_cliente(void* arg) {
    // Extraer el File Descriptor del cliente y liberar la memoria alocada
    int client_fd = *((int*)arg);
    free(arg);

    char buffer[256];
    memset(buffer, 0, sizeof(buffer));

    // Leer el comando enviado por el cliente
    ssize_t bytes_leidos = read(client_fd, buffer, sizeof(buffer) - 1);
    if (bytes_leidos > 0) {
        // Limpiar saltos de línea por si el cliente los envía
        buffer[strcspn(buffer, "\r\n")] = 0;
        
        char respuesta[256];
        memset(respuesta, 0, sizeof(respuesta));

        // 1. Bloquear Mutex antes de tocar el hardware y el estado global
        pthread_mutex_lock(&mutex_led);

        // 2 & 3. Evaluar comando, interactuar con GPIO y actualizar estado
        if (strcmp(buffer, "ON") == 0) {
            led_estado = 1;
            if (!modo_simulado) gpioWrite(LED_PIN, 1);
            strcpy(respuesta, "LED_OK: ON");
            printf("[Servidor] Comando recibido: ON. LED Encendido.\n");

        } else if (strcmp(buffer, "OFF") == 0) {
            led_estado = 0;
            if (!modo_simulado) gpioWrite(LED_PIN, 0);
            strcpy(respuesta, "LED_OK: OFF");
            printf("[Servidor] Comando recibido: OFF. LED Apagado.\n");

        } else if (strcmp(buffer, "STATUS") == 0) {
            sprintf(respuesta, "ESTADO ACTUAL: %s", led_estado ? "ON" : "OFF");
            printf("[Servidor] Comando recibido: STATUS. Reportando estado.\n");

        } else {
            strcpy(respuesta, "ERROR: Comando no reconocido (Usa ON, OFF o STATUS)");
            printf("[Servidor] Comando invalido recibido: %s\n", buffer);
        }

        // 4. Liberar Mutex inmediatamente después del trabajo crítico
        pthread_mutex_unlock(&mutex_led);

        // 5. Enviar confirmación al cliente
        write(client_fd, respuesta, strlen(respuesta));
    }

    // Cerrar conexión con este cliente específico y terminar hilo
    close(client_fd);
    return NULL;
}

// =========================================================================
// MAIN: Gestor de Sockets (Hilo Principal)
// =========================================================================
int main() {
    printf("Iniciando Servidor de Control de Actuadores (Daemon)...\n");

    // Inicialización del Hardware
    if (gpioInitialise() < 0) {
        printf("ADVERTENCIA: No se pudo inicializar pigpio (¿Faltó sudo? o ¿No es una Raspberry Pi?).\n");
        printf("Iniciando en MODO SIMULADO. El servidor aceptará comandos pero no activará pines reales.\n");
        modo_simulado = 1;
    } else {
        gpioSetMode(LED_PIN, PI_OUTPUT);
        gpioWrite(LED_PIN, 0); // Apagar LED por defecto
    }

    int server_fd;
    struct sockaddr_un addr;

    // Borrar el archivo de socket si quedó colgado de una ejecución anterior
    unlink(SOCKET_PATH);

    // Crear el socket de flujo en el dominio Unix
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Error al crear el socket");
        return 1;
    }

    // Configurar la dirección del socket Unix
    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    // Enlazar (Bind) el socket al archivo /tmp/control_led.sock
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("Error en bind");
        return 1;
    }

    // Escuchar conexiones (hasta 10 en la fila de espera)
    if (listen(server_fd, 10) == -1) {
        perror("Error en listen");
        return 1;
    }

    printf("Servidor escuchando en %s\n", SOCKET_PATH);

    // Bucle infinito: Aceptar clientes
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            perror("Error aceptando conexion");
            continue;
        }

        // Alocar memoria para pasar el FD al hilo de forma segura
        int* pclient = malloc(sizeof(int));
        *pclient = client_fd;

        pthread_t tid;
        if (pthread_create(&tid, NULL, atender_cliente, pclient) != 0) {
            perror("Error creando hilo worker");
            free(pclient);
            close(client_fd);
        } else {
            // Desvincular el hilo para que el SO recoja sus recursos al terminar (sin hacer join)
            pthread_detach(tid);
        }
    }

    // Limpieza (Inalcanzable en este código continuo, pero correcto formalmente)
    if (!modo_simulado) gpioTerminate();
    close(server_fd);
    unlink(SOCKET_PATH);
    return 0;
}