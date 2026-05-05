#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pigpio.h>
#include <string.h>

// =========================================================================
// 1. CONFIGURACIÓN Y CONSTANTES
// =========================================================================
// MPU6050
#define MPU6050_ADDR 0x68
#define PWR_MGMT_1   0x6B
#define ACCEL_XOUT_H 0x3B

// Cola de mensajes POSIX
#define NOMBRE_COLA "/mq_vibracion"

// Configuración del filtro de media móvil
#define N_MUESTRAS 5  // Cantidad de muestras a promediar

// Estructura de datos para la terna inercial
typedef struct {
    float x;
    float y;
    float z;
} TernaAceleracion;

// =========================================================================
// 2. VARIABLES GLOBALES
// =========================================================================
int i2c_handle;
mqd_t mq;
volatile int corriendo = 1;

// =========================================================================
// 3. HILO A: PRODUCTOR (Adquisición MPU6050) - 100Hz
// =========================================================================
void* hilo_productor(void* arg) {
    char buf[6];
    TernaAceleracion data_cruda;
    
    // Configuración del sensor: Despertar de modo Sleep enviando un 0 al registro de energía
    if (i2cWriteByteData(i2c_handle, PWR_MGMT_1, 0x00) != 0) {
        fprintf(stderr, "Error: No se pudo despertar el MPU6050.\n");
        corriendo = 0;
        return NULL;
    }
    
    fprintf(stderr, "[Productor] Sensor inicializado. Capturando a 100Hz...\n");

    while (corriendo) {
        // Leer 6 bytes comenzando desde el registro de aceleración X
        if (i2cReadI2CBlockData(i2c_handle, ACCEL_XOUT_H, buf, 6) == 6) {
            // El MPU6050 devuelve datos en complemento a 2 de 16 bits (Parte Alta y Parte Baja)
            int16_t ax = (buf[0] << 8) | buf[1];
            int16_t ay = (buf[2] << 8) | buf[3];
            int16_t az = (buf[4] << 8) | buf[5];

            // Convertir a Gravedades ('g') usando el divisor estándar de 16384.0 para escala +/- 2g
            data_cruda.x = ax / 16384.0f;
            data_cruda.y = ay / 16384.0f;
            data_cruda.z = az / 16384.0f;

            // Enviar la terna por la cola de mensajes POSIX
            if (mq_send(mq, (const char*)&data_cruda, sizeof(TernaAceleracion), 0) == -1) {
                fprintf(stderr, "[Productor] Error al enviar a la cola.\n");
            }
        }
        
        // Frecuencia solicitada: 100 Hz = 10ms = 10000 us
        usleep(10000); 
    }
    return NULL;
}

// =========================================================================
// 4. HILO B: CONSUMIDOR (Procesamiento y Salida)
// =========================================================================
void* hilo_consumidor(void* arg) {
    TernaAceleracion data_recibida;
    
    // Buffer circular para el filtro de media móvil
    TernaAceleracion historia[N_MUESTRAS] = {0};
    int indice = 0;
    int conteo = 0; // Para saber si ya llenamos la ventana inicial

    fprintf(stderr, "[Consumidor] Cola abierta. Esperando datos...\n");

    while (corriendo) {
        // Recibir datos de la cola (Llamada bloqueante)
        ssize_t bytes_leidos = mq_receive(mq, (char*)&data_recibida, sizeof(TernaAceleracion), NULL);
        
        if (bytes_leidos == sizeof(TernaAceleracion)) {
            // Guardar en el buffer circular
            historia[indice] = data_recibida;
            indice = (indice + 1) % N_MUESTRAS;
            if (conteo < N_MUESTRAS) conteo++; // Llenado inicial del filtro

            // Calcular el promedio (Filtro de Media Móvil)
            float sum_x = 0.0f, sum_y = 0.0f, sum_z = 0.0f;
            for (int i = 0; i < conteo; i++) {
                sum_x += historia[i].x;
                sum_y += historia[i].y;
                sum_z += historia[i].z;
            }
            float filt_x = sum_x / conteo;
            float filt_y = sum_y / conteo;
            float filt_z = sum_z / conteo;

            // SALIDA LIMPIA (Requerimiento 3 y 5.3):
            // Obligatorio imprimir a stdout en formato "x,y,z" seguido de salto de línea
            printf("%.3f,%.3f,%.3f\n", filt_x, filt_y, filt_z);
            
            // CRÍTICO para los Pipes (Tuberías) en Linux:
            // Forzar el vaciado del buffer para que Python lo lea en tiempo real
            fflush(stdout); 
        }
    }
    return NULL;
}

// =========================================================================
// 5. MAIN (Orquestación)
// =========================================================================
int main() {
    fprintf(stderr, "===============================================\n");
    fprintf(stderr, "Iniciando Medidor de Vibraciones I2C -> Pipe\n");
    fprintf(stderr, "===============================================\n");

    // Inicializar Pigpio para el I2C
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Error Fatal: No se pudo inicializar pigpio (¿Faltó sudo?).\n");
        return 1;
    }

    // Abrir bus I2C 1, dirección 0x68
    i2c_handle = i2cOpen(1, MPU6050_ADDR, 0);
    if (i2c_handle < 0) {
        fprintf(stderr, "Error Fatal: No se detecta sensor en 0x68.\n");
        gpioTerminate();
        return 1;
    }

    // Configurar atributos de la Cola de Mensajes POSIX
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;                     // Profundidad máxima de 10 mensajes (100ms de atraso max)
    attr.mq_msgsize = sizeof(TernaAceleracion);  // Tamaño exacto de la estructura
    attr.mq_curmsgs = 0;

    // Desvincular cola vieja por si el programa crasheó antes
    mq_unlink(NOMBRE_COLA);
    
    // Crear nueva cola con acceso Lectura/Escritura
    mq = mq_open(NOMBRE_COLA, O_CREAT | O_RDWR, 0644, &attr);
    if (mq == (mqd_t)-1) {
        fprintf(stderr, "Error Fatal: No se pudo crear la cola POSIX.\n");
        i2cClose(i2c_handle);
        gpioTerminate();
        return 1;
    }

    // Creación de hilos
    pthread_t th_productor, th_consumidor;
    pthread_create(&th_consumidor, NULL, hilo_consumidor, NULL);
    pthread_create(&th_productor, NULL, hilo_productor, NULL);

    // Bucle eterno (Terminar con Ctrl+C)
    while(1) {
        sleep(1);
    }

    // Limpieza general (Inalcanzable sin modificar señales, pero buena práctica)
    corriendo = 0;
    pthread_join(th_productor, NULL);
    pthread_join(th_consumidor, NULL);
    mq_close(mq);
    mq_unlink(NOMBRE_COLA);
    i2cClose(i2c_handle);
    gpioTerminate();
    
    return 0;
}
