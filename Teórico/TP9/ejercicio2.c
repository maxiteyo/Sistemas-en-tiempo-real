#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#define LED_TAREA1 17
#define LED_TAREA2 27
#define LED_TAREA3 22

// Variable global para contar el tiempo en ms
volatile unsigned long ticks_global_ms = 0;
volatile int tick_flag = 0;

void tickHandler(int sig) {
    ticks_global_ms += 100; // Incrementamos de a 100 ms que será el tick base
    tick_flag = 1;
}

void configTimer() {
    timer_t timerid;
    struct sigevent sev;
    struct itimerspec its;
  
    signal(SIGALRM, tickHandler); 

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGALRM;
    sev.sigev_value.sival_ptr = &timerid;
    if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
        perror("Error en timer_create");

    // Configurar temporizador para dispararse cada 100 ms (100,000,000 ns)
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 100000000;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 100000000;

    if (timer_settime(timerid, 0, &its, NULL) == -1)
        perror("Error en timer_settime");
}

unsigned long get_tics_ms() {
    return ticks_global_ms;
}

// Tarea 1: Período de 100 ms
void Task1(void) {
    static int state = 0;
    static unsigned long timer;
    static int led_state = 0;
    
    unsigned long Tics = get_tics_ms();

    switch (state) {
        case 0:
            timer = Tics + 100; 
            state = 1;
            break;
        case 1:
            if (Tics >= timer) {
                led_state = !led_state; // Invertir estado
                printf("[Tarea 1 - LED %d] - %s - Tics: %lu ms\n", LED_TAREA1, led_state ? "ON " : "OFF", Tics);
                timer = Tics + 100;
            }
            break;
    }
}

// Tarea 2: Período de 300 ms
void Task2(void) {
    static int state = 0;
    static unsigned long timer;
    static int led_state = 0;
    
    unsigned long Tics = get_tics_ms();

    switch (state) {
        case 0:
            timer = Tics + 300;
            state = 1;
            break;
        case 1:
            if (Tics >= timer) {
                led_state = !led_state;
                printf("[Tarea 2 - LED %d] - %s - Tics: %lu ms\n", LED_TAREA2, led_state ? "ON " : "OFF", Tics);
                timer = Tics + 300;
            }
            break;
    }
}

// Tarea 3: Período de 500 ms
void Task3(void) {
    static int state = 0;
    static unsigned long timer;
    static int led_state = 0;
    
    unsigned long Tics = get_tics_ms();

    switch (state) {
        case 0:
            timer = Tics + 500;
            state = 1;
            break;
        case 1:
            if (Tics >= timer) {
                led_state = !led_state;
                printf("[Tarea 3 - LED %d] - %s - Tics: %lu ms\n", LED_TAREA3, led_state ? "ON " : "OFF", Tics);
                timer = Tics + 500;
            }
            break;
    }
}

int main() {
    printf("Iniciando Ejecutivo Cíclico simulado con POSIX Timers...\n");

    configTimer();

    // Súper-bucle infinito del ejecutivo cíclico
    while (1) {
        if (tick_flag) {
            tick_flag = 0;
            // Evaluamos las tareas en cada tick
            Task1();
            Task2();
            Task3();
        }
        
        // Esperamos pasivamente al próximo tick en lugar de quemar CPU o usar sleep
        pause(); 
    }

    return 0;
}
