#define _GNU_SOURCE  

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <sched.h>

volatile int running = 1;
unsigned long cont_critica = 0;
unsigned long cont_navegacion = 0;
unsigned long cont_telemetria = 0;

char modo_ensayo = 'B'; // Por defecto Caso B (FIFO normal)
pthread_mutex_t recurso_compartido = PTHREAD_MUTEX_INITIALIZER;
sigset_t mascara_sig;

// Función para "Quemar CPU" y simular trabajo pesado matemático (SIN SLEEP)
void trabajo_pesado(int ciclos) {
    volatile double calculo = 1.0;
    for (int i = 0; i < ciclos; i++) {
        calculo = calculo * 1.000001; 
    }
}

// -------------------------------------------------------------
// 1. Hilo Crítico (Control de Estabilidad) - Prioridad 80
// -------------------------------------------------------------
void* tarea_critica(void* arg) {
    while (running) {
        if (modo_ensayo == 'C') pthread_mutex_lock(&recurso_compartido);
        
        trabajo_pesado(500000); // Trabajo pesado de CPU
        cont_critica++;
        
        if (modo_ensayo == 'C') pthread_mutex_unlock(&recurso_compartido);
    }
    return NULL;
}

// -------------------------------------------------------------
// 2. Hilo Medio (Navegación) - Prioridad 40
// -------------------------------------------------------------
void* tarea_navegacion(void* arg) {
    while (running) {
        // En el Caso C, este hilo NO usa el mutex, se dedica unicamente a 
        // acaparar la CPU para que el Hilo Bajo nunca pueda destrabarse.
        trabajo_pesado(500000); 
        cont_navegacion++;
    }
    return NULL;
}

// -------------------------------------------------------------
// 3. Hilo Bajo (Telemetría) - Prioridad 10
// -------------------------------------------------------------
void* tarea_telemetria(void* arg) {
    int sig_recibida;
    while (running) {
        if (modo_ensayo == 'C') {
            // En Caso C simulamos que usa el recurso compartido largo tiempo
            pthread_mutex_lock(&recurso_compartido);
            trabajo_pesado(10000000); // Operación larguísima bloqueando el candado
            pthread_mutex_unlock(&recurso_compartido);
        } else {
            // Requisito del TP (Casos A y B): Esperar 500ms exactos vía sigwait
            sigwait(&mascara_sig, &sig_recibida);
            // printf("[TEL] Log a tierra (500ms)\n"); // Comentado para no saturar consola 10 segundos
        }
        
        cont_telemetria++;
    }
    return NULL;
}

// -------------------------------------------------------------
// Configurador del Timer POSIX
// -------------------------------------------------------------
void setup_timer() {
    sigemptyset(&mascara_sig);
    sigaddset(&mascara_sig, SIGALRM);
    
    // Bloqueamos la señal en el MAIN (Los hilos heredarán esto). 
    // Así evitamos paros asíncronos y forzamos a que solo "sigwait" lo lea.
    sigprocmask(SIG_BLOCK, &mascara_sig, NULL);

    struct sigevent sev;
    timer_t timerid;
    struct itimerspec its;

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    sev.sigev_value.sival_ptr = &timerid;
    timer_create(CLOCK_REALTIME, &sev, &timerid);

    // Disparo a los 500ms (500.000.000 nanosegundos)
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 500000000;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 500000000;
    timer_settime(timerid, 0, &its, NULL);
}

// ==============================================================
// MAIN: Orquestador y planificador de 10 segundos
// ==============================================================
int main(int argc, char *argv[]) {
    // Lectura del parámetro A, B o C
    if (argc > 1) {
        modo_ensayo = argv[1][0];
    } else {
        printf("Uso incorrecto. Ejecuta: sudo ./simulador_vuelo [A/B/C]\n");
        return 1;
    }

    printf("\n========================================\n");
    printf(" SIMULADOR DE VUELO - MODO ENSAYO: %c\n", modo_ensayo);
    printf("========================================\n");

    // === MAGIA PARA PCs MODERNAS (CPU Affinity) ===
    // Para poder observar cómo los hilos se "roban" el recurso y pelean por él,
    // obligamos a Linux a correr todos los hilos de este programa en el NUCLEO 0.
    // De lo contrario, en tu PC multinúcleo, cada hilo tomaría un núcleo diferente
    // y correrían en paralelo sin estorbarse, arruinando el experimento.
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);

    setup_timer();

    // Como main correrá a 10s, forzamos que tenga la MÁXIMA PRIORIDAD POSIBLE (99)
    // De otra forma, si tenemos una máquina lenta, los hilos de CPU pueden "Ahogar" al
    // hilo main y la prueba de 10 segundos se congelaría para siempre.
    if (modo_ensayo != 'A') {
        struct sched_param param_main;
        param_main.sched_priority = 99; 
        pthread_setschedparam(pthread_self(), SCHED_FIFO, &param_main);
    }

    pthread_t th_critica, th_nav, th_tel;
    pthread_attr_t attr_c, attr_n, attr_t;
    struct sched_param param_c, param_n, param_t;

    pthread_attr_init(&attr_c);
    pthread_attr_init(&attr_n);
    pthread_attr_init(&attr_t);

    if (modo_ensayo == 'A') {
        printf(" -> Politica: SCHED_OTHER (Pelea justa genérica por CPU del Linux)\n");
        // No configuramos prioridades, dejamos las estándar de Windows/Linux.
    } else {
        printf(" -> Politica: SCHED_FIFO (Prioridad estricta de hardware)\n");
        
        pthread_attr_setinheritsched(&attr_c, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&attr_c, SCHED_FIFO);
        param_c.sched_priority = 80;
        pthread_attr_setschedparam(&attr_c, &param_c);

        pthread_attr_setinheritsched(&attr_n, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&attr_n, SCHED_FIFO);
        param_n.sched_priority = 40;
        pthread_attr_setschedparam(&attr_n, &param_n);

        pthread_attr_setinheritsched(&attr_t, PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&attr_t, SCHED_FIFO);
        param_t.sched_priority = 10;
        pthread_attr_setschedparam(&attr_t, &param_t);
    }

    // Orden CRÍTICO de cración de Hilos para forzar el Caso C (Inversión):
    // 1. Arrancamos Hilo Bajo primero, para darle un microsegundo a que agarre el Mutex.
    int status = pthread_create(&th_tel, &attr_t, tarea_telemetria, NULL);
    if(status != 0) {
        printf("Error: Fallo al crear los hilos RT. ¿Olvidaste usar 'sudo'?\n"); return 1;
    }
    usleep(50000); 

    // 2. Arrancamos Hilo Medio. Al ser mayor, aplasta al hilo bajo.
    pthread_create(&th_nav, &attr_n, tarea_navegacion, NULL);
    usleep(50000);

    // 3. Arrancamos Hilo Alto. Intenta agarrar Mutex, pero como el Bajo lo tiene y el Medio no lo deja hablar, se atasca.
    pthread_create(&th_critica, &attr_c, tarea_critica, NULL);

    // Cronometramos los 10 Segundos solicitados
    printf("Corriendo iteraciones matemáticas por 10 segundos...\n");
    sleep(10);
    running = 0; // Bandera bajada, frenamos los while() de la CPU

    // Inyectamos una falsa alarma para despertar a telemetría si se quedó congelada en sigwait()
    pthread_kill(th_tel, SIGALRM);

    pthread_join(th_critica, NULL);
    pthread_join(th_nav, NULL);
    pthread_join(th_tel, NULL);

    // Resultos finales = Tiempos de iteraciones de CPU
    printf("\n------------- METRICAS FINALES EN 10 SEGUNDOS -------------\n");
    if(modo_ensayo == 'A') {
        printf("Critica (Normal) : %lu iteraciones\n", cont_critica);
        printf("Navegacion (Normal): %lu iteraciones\n", cont_navegacion);
        printf("Telemetria (Normal): %lu iteraciones (Espera sus ~20 ciclos estandar)\n", cont_telemetria);
    } else {
        printf("Critica    [Prio 80]: %lu iteraciones\n", cont_critica);
        printf("Navegacion [Prio 40]: %lu iteraciones\n", cont_navegacion);
        printf("Telemetria [Prio 10]: %lu iteraciones\n", cont_telemetria);
    }
    printf("-----------------------------------------------------------\n\n");

    return 0;
}