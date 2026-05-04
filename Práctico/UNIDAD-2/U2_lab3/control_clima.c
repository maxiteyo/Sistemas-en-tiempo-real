#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <pigpio.h>
#include <stdint.h>
#include <sched.h>

// =========================================================================
// 1. CONSTANTES Y CONFIGURACIÓN HARDWARE / LÓGICA
// =========================================================================
#define AHT10_ADDR 0x38     // Dirección I2C estándar del sensor de temp/hum
#define FAN_PIN 17          // GPIO 17 para el actuador (ventilador/LED)

// Umbrales adaptados para Ensayo Físico en Laboratorio (Calor corporal/aliento)
#define TEMP_ALERTA 28.0    // Temperatura para entrar en estado de ALERTA (Fácil de llegar con la mano/aliento)
#define TEMP_REPOSO 26.0    // Temperatura para retornar a REPOSO y apagar ventilador

// Tiempos definidos mediante la consigna (en microsegundos para gpioTick)
// Nota: Se redujo a 10 seg el tiempo continuo de alerta para no esperar 60s en la demostración en vivo.
#define TIEMPO_ALERTA_US      10000000  // 10 Segundos (Consigna teórica: 60s)
#define TIEMPO_VENTILACION_US 120000000 // 120 Segundos máximos (Consigna teórica: 120s)

// Estados de la Máquina de Estados
typedef enum {
    REPOSO,
    ALERTA,
    VENTILACION
} EstadoSistema;

// =========================================================================
// 2. VARIABLES GLOBALES Y RECURSOS COMPARTIDOS (Protegidos por Mutex)
// =========================================================================
float temperatura_global = 20.0;
pthread_mutex_t mutex_temp = PTHREAD_MUTEX_INITIALIZER; // Mutex POSIX

// Variables de estado del sistema (Solo leídas/escritas por Tarea B y C)
EstadoSistema estado_sistema = REPOSO;
int ventilador_encendido = 0;
uint32_t tiempo_inicio_sistema;

int i2c_handle; // Manejador del bus I2C

// =========================================================================
// 3. TAREA A: Adquisición de Datos I2C (Prioridad 80 - SCHED_FIFO)
// =========================================================================
void* tarea_adquisicion(void* arg) {
    char init_cmd[3] = {0xE1, 0x08, 0x00};
    i2cWriteDevice(i2c_handle, init_cmd, 3);
    sleep(1); // Esperar calibración del AHT10

    while (1) {
        // Disparar medición en el AHT10
        char trigger_cmd[3] = {0xAC, 0x33, 0x00};
        i2cWriteDevice(i2c_handle, trigger_cmd, 3);
        
        // Esperar el tiempo de conversión de hardware (~75ms)
        usleep(80000); 
        
        char data[6];
        int bytes_read = i2cReadDevice(i2c_handle, data, 6);
        
        // Si se leyeron 6 bytes y el sensor no está ocupado (Bit 7 en 0)
        if (bytes_read == 6 && (data[0] & 0x80) == 0) { 
            // Matemática de conversión según datasheet del AHT10
            uint32_t raw_temp = ((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5];
            float temp_calc = (raw_temp * 200.0 / 1048576.0) - 50.0;
            
            // --- SECCIÓN CRÍTICA ---
            // Regla de oro de la consigna: El I2C ya se leyó, el Mutex solo encierra 
            // la actualización de la variable en memoria RAM (Extremadamente fugaz)
            pthread_mutex_lock(&mutex_temp);
            temperatura_global = temp_calc;
            pthread_mutex_unlock(&mutex_temp);
            // -----------------------
        }
        
        sleep(1); // Lectura cada 1 segundo (Consigna)
    }
    return NULL;
}

// =========================================================================
// 4. TAREA B: Control Lógico / Máquina de Estados (Prioridad 40 - SCHED_FIFO)
// =========================================================================
void* tarea_control(void* arg) {
    uint32_t tick_inicio_alerta = 0;
    uint32_t tick_inicio_ventilacion = 0;

    while (1) {
        float temp_local;
        
        // --- SECCIÓN CRÍTICA ---
        // Extraemos una copia fugaz para usarla libremente sin retener el Mutex
        pthread_mutex_lock(&mutex_temp);
        temp_local = temperatura_global;
        pthread_mutex_unlock(&mutex_temp);
        // -----------------------
        
        uint32_t tick_actual = gpioTick(); // Reloj de alta precisión del sistema

        switch(estado_sistema) {
            case REPOSO:
                if (temp_local > TEMP_ALERTA) {
                    estado_sistema = ALERTA;
                    tick_inicio_alerta = tick_actual; // Arranca cronómetro continuo
                }
                break;

            case ALERTA:
                if (temp_local <= TEMP_ALERTA) {
                    estado_sistema = REPOSO; // Falsa alarma o pico aislado, vuelve a cero
                } else {
                    // Si pasaron los X segundos exigidos de forma continua
                    if ((tick_actual - tick_inicio_alerta) >= TIEMPO_ALERTA_US) {
                        estado_sistema = VENTILACION;
                        tick_inicio_ventilacion = tick_actual;
                        ventilador_encendido = 1;
                        gpioWrite(FAN_PIN, 1); // Encendemos hardware
                    }
                }
                break;

            case VENTILACION:
                // Si la temperatura baja del umbral de reposo O se vence el tiempo max(120s)
                if (temp_local < TEMP_REPOSO || (tick_actual - tick_inicio_ventilacion) >= TIEMPO_VENTILACION_US) {
                    estado_sistema = REPOSO;
                    ventilador_encendido = 0;
                    gpioWrite(FAN_PIN, 0); // Apagamos hardware
                }
                break;
        }
        
        usleep(100000); // Polling eficiente cada 100ms para no saturar la CPU
    }
    return NULL;
}

// =========================================================================
// 5. TAREA C: Interfaz y Diagnóstico (Prioridad Estándar - SCHED_OTHER)
// =========================================================================
void* tarea_diagnostico(void* arg) {
    while (1) {
        float temp_local;
        
        // --- SECCIÓN CRÍTICA ---
        pthread_mutex_lock(&mutex_temp);
        temp_local = temperatura_global;
        pthread_mutex_unlock(&mutex_temp);
        // -----------------------
        
        uint32_t uptime_seg = (gpioTick() - tiempo_inicio_sistema) / 1000000;
        
        const char* nom_estado = (estado_sistema == REPOSO) ? "REPOSO     " : 
                                 (estado_sistema == ALERTA) ? "ALERTA     " : "VENTILACIÓN";
                                 
        // Regla de oro: NUNCA hacer un printf dentro de un mutex. 
        printf("[Uptime: %03d s] Temp: %.2f °C | Estado: %s | Ventilador: %s\n",
               uptime_seg, temp_local, nom_estado, ventilador_encendido ? "ENCENDIDO" : "APAGADO");
               
        sleep(5); // Interfaz de usuario cada 5 segundos (Consigna)
    }
    return NULL;
}

// =========================================================================
// MAIN: Orquestador y Configuración POSIX Real-Time
// =========================================================================
int main() {
    printf("Inicializando Control de Clima (I2C AHT10)...\n");

    // Integridad Hardware 1: pigpio
    if (gpioInitialise() < 0) {
        printf("Error Fatal: No se pudo inicializar pigpio. (¿Faltó sudo?)\n");
        return 1;
    }
    
    // Integridad Hardware 2: Apertura I2C Bus 1
    i2c_handle = i2cOpen(1, AHT10_ADDR, 0);
    if (i2c_handle < 0) {
        printf("Error Fatal: No se pudo abrir el bus I2C. Revisar conexiones del AHT10.\n");
        gpioTerminate();
        return 1;
    }
    
    // Setup Actuador
    gpioSetMode(FAN_PIN, PI_OUTPUT);
    gpioWrite(FAN_PIN, 0);
    
    tiempo_inicio_sistema = gpioTick();

    // Hilos y Atributos POSIX
    pthread_t th_a, th_b, th_c;
    pthread_attr_t attr_a, attr_b, attr_c;
    struct sched_param param_a, param_b;

    // Tarea A (RT: FIFO Prio 80)
    pthread_attr_init(&attr_a);
    pthread_attr_setinheritsched(&attr_a, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr_a, SCHED_FIFO);
    param_a.sched_priority = 80;
    pthread_attr_setschedparam(&attr_a, &param_a);

    // Tarea B (RT: FIFO Prio 40)
    pthread_attr_init(&attr_b);
    pthread_attr_setinheritsched(&attr_b, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr_b, SCHED_FIFO);
    param_b.sched_priority = 40;
    pthread_attr_setschedparam(&attr_b, &param_b);

    // Tarea C (General: OTHER por defecto)
    pthread_attr_init(&attr_c); 

    // Lanzamiento Concurrente
    if (pthread_create(&th_a, &attr_a, tarea_adquisicion, NULL) != 0 ||
        pthread_create(&th_b, &attr_b, tarea_control, NULL) != 0 ||
        pthread_create(&th_c, &attr_c, tarea_diagnostico, NULL) != 0) {
        printf("Error al crear hilos RT. Requiere permisos sudo.\n");
        return 1;
    }

    // Mantener proceso vivo
    pthread_join(th_a, NULL);
    pthread_join(th_b, NULL);
    pthread_join(th_c, NULL);

    i2cClose(i2c_handle);
    gpioTerminate();
    return 0;
}
