#define _POSIX_C_SOURCE 199309L
#define _XOPEN_SOURCE 500
#define _DEFAULT_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sched.h>
#include <sys/time.h>
#include <pigpio.h> // LIBRERIA REAL AGREGADA PARA CONTROL DE PLACA


// -------------------------------------------------------------
// FUNCIONES DE HARDWARE REAL CON PIGPIO
// -------------------------------------------------------------
#define PIN_SERVO  18
#define PIN_BOTON  2
#define PIN_LED    3   


int leer_boton() {
   return gpioRead(PIN_BOTON); // Lee el pin (devuelve 1 si entra tension, asume logica positiva)
}


void encender_led_advertencia() {
   gpioWrite(PIN_LED, 1); // Envia 3.3v al pin del LED
}


void mover_servo(int grados) {
   // Para servos sg90/mg90s estandar, pigpio usa ancho de pulso:
   // 500 us (0 grados) a 2500 us (180 grados).
   int pulse = 500 + (grados * 2000 / 180);
   gpioServo(PIN_SERVO, pulse);
}


// -------------------------------------------------------------
// VARIABLES GLOBALES COMPARTIDAS (Estado del sistema)
// -------------------------------------------------------------
volatile int posicion_servo = 0;
volatile int estado_alerta = 0;    // 0 = Funcionamiento Normal, 1 = Alerta!


// ==============================================================
// TAREA 3: TELEMETRIA (Timer de Tiempo Real)
// ==============================================================
// Esta funcion se dispara automáticamente por interrupción del SO.
// Por eso no necesita sleep() ni estar en un bucle while propio.
void telemetria_handler(int sig, siginfo_t *si, void *uc) {
   // Concepto clave: El timer interrumpe lo que sea que esté haciendo
   // la CPU y ejecuta esta función cada 1 segundo clavado.
   printf("[TELEMETRIA] Posicion Servo: %3d° | Estado del Sistema: [%s]\n",
          posicion_servo, estado_alerta ? "ALERTA/PARADA" : "SEGURO/MOVIENDO");
}


void setup_realtime_timer() {
   struct sigaction sa;
   struct sigevent sev;
   timer_t timerid;
   struct itimerspec its;


   // Configura la acción para capturar la Señal de Tiempo Real (SIGRTMIN)
   sa.sa_flags = SA_SIGINFO; 
   sa.sa_sigaction = telemetria_handler;
   sigemptyset(&sa.sa_mask); 
   sigaction(SIGRTMIN, &sa, NULL); 


   // Crear el Timer y vincularlo a esa señal
   sev.sigev_notify = SIGEV_SIGNAL;
   sev.sigev_signo = SIGRTMIN;
   sev.sigev_value.sival_ptr = &timerid;
   timer_create(CLOCK_REALTIME, &sev, &timerid);


   // Configurar interrupciones exactas de 1.0 segundos
   its.it_value.tv_sec = 1;
   its.it_value.tv_nsec = 0;
   its.it_interval.tv_sec = 1;      // Intervalo de repetición (1s)
   its.it_interval.tv_nsec = 0;
  
   // Iniciar timer
   timer_settime(timerid, 0, &its, NULL);
}
 

// ==============================================================
// TAREA 2: MONITOREO DE SEGURIDAD (Hilo POSIX RT - SCHED_FIFO)
// ==============================================================
void* tarea_monitoreo(void* arg) {
   // Este hilo corre de fondo.
   int estado_boton_anterior = 1; // 1 porque el boton suelto da 1 (Pull-Up)

   while (1) {
       int estado_boton_actual = leer_boton();

       // Detectar el momento EXACTO en el que se aprieta el boton (Flanco de bajada: 1 -> 0)
       if (estado_boton_actual == 0 && estado_boton_anterior == 1) {
           
           if (estado_alerta == 0) { 
               // 1. Entramos en modo emergencia
               estado_alerta = 1;    // Frenar la carga principal
               encender_led_advertencia(); // Activar actuador de emergencia
              
               printf("\n======================================================\n");
               printf("[ALERTA] Parada de emergencia activada - Latencia detectada.\n");
               printf("======================================================\n\n");
           } else {
               // 2. Ya estabamos en alerta, asi que "Reanudamos"
               estado_alerta = 0;     // Permite volver a mover el servo
               gpioWrite(PIN_LED, 0); // Apagar LED de advertencia
               
               printf("\n======================================================\n");
               printf("[INFO] Sistema normalizado. Reanudando movimiento...\n");
               printf("======================================================\n\n");
           }

           // Efecto Anti-rebote (Debounce)
           usleep(300000); // Congelar lectura 300ms para que el dedo del humano no active esto múltiples veces
       }

       estado_boton_anterior = estado_boton_actual;
       usleep(5000); // Polling muy rápido (cada 5 ms) para baja latencia
   }
   return NULL;
}


// ==============================================================
// MAIN / TAREA 1: EJECUTIVO CÍCLICO Y CARGA PRINCIPAL
// ==============================================================
int main() {
   printf("Iniciando Sistema de Barrera Automatica Multitarea...\n\n");


   // Inicializamos pigpio real
   if (gpioInitialise() < 0) {
       fprintf(stderr, "Error al inicializar pigpio. Recuerda ejecutar con 'sudo'.\n");
       return 1;
   }


   // Configuracion de pines de hardware
   gpioSetMode(PIN_BOTON, PI_INPUT);
   gpioSetPullUpDown(PIN_BOTON, PI_PUD_UP); // Resistencia interna hacia 3.3V (Pull-Up)
   gpioSetMode(PIN_LED, PI_OUTPUT);
   gpioWrite(PIN_LED, 0); // Comenzamos con el LED de advertencia apagado


   // 1) LANZAR TAREA 3: Timer Píoloto (Dispara interrupciones)
   setup_realtime_timer();


   // 2) LANZAR TAREA 2: Hilo paralelo para Botón con PRIORIDAD RT
   pthread_t hilo_boton;
   pthread_attr_t attr;
   struct sched_param param;


   pthread_attr_init(&attr);
   // Configurar el tipo de planificación (Real Time: FIFO)
   pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
   pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
   param.sched_priority = 50; // La máxima recomendada (entre 1 y 99)
   pthread_attr_setschedparam(&attr, &param);


   // Intentamos crear el hilo con permisos RT (requiere ser Root/Sudo)
   int rt_status = pthread_create(&hilo_boton, &attr, tarea_monitoreo, NULL);
   if (rt_status != 0) {
       printf("Aviso: No se pudo asignar SCHED_FIFO (ejecuta con 'sudo').\nLevantando hilo con prioridad normal...\n\n");
       // Si no tenemos permisos (fallo de root), lo corremos como hilo normal para probar
       pthread_create(&hilo_boton, NULL, tarea_monitoreo, NULL);
   }
   pthread_attr_destroy(&attr);




   // 3) CARGA PRINCIPAL: Movimiento suave del Servo
   int direccion = 1; // 1 = subiendo grados, -1 = restando grados


   while (1) {
       // En caso de ALERTA, nos salteamos actualizar posición (Efecto freno!)
       if (estado_alerta == 0) {
          
           // Lógica cíclica de vaivén 0 a 180
           posicion_servo += direccion;
           if (posicion_servo >= 180) {
               posicion_servo = 180;
               direccion = -1;
           } else if (posicion_servo <= 0) {
               posicion_servo = 0;
               direccion = 1;
           }
          
           // Físicamente mover el servo a los grados deseados
           mover_servo(posicion_servo);
       }


       // Movimiento fluido. En un servo real, cada giro de 1° a esta velocidad toma un tiempo.
       usleep(20000); // 20ms entre grado y grado = Movimiento elegante
   }


   gpioTerminate();
   return 0;
}


