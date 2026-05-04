# Actividad Práctica de Aula n.º 4 - Manejo de Procesos con POSIX en C

Este documento contiene las instrucciones de compilación y ejecución para los ejercicios correspondientes al Trabajo Práctico 4 (Procesos, `fork` y `exec` de la familia POSIX).

---

## Punto 2: Generación de 10 Hilos de Procesos Hijos (Fork)
Este programa crea 10 procesos hijos derivados del padre. Cada hijo se "duerme" 15 segundos para dar tiempo a inspeccionarlos en el sistema operativo mediante el árbol de procesos.

**1. Compilación:**
```bash
gcc ejercicio2.c -o ejercicio2
```

**2. Ejecución:**
En la primera terminal, ejecutá el programa:
```bash
./ejercicio2
```

**3. Comprobación (Rápido, en los primeros 15 segundos):**
En una SEGUNDA terminal, ejecutá el siguiente comando para ver gráficamente cómo el SO muestra al proceso padre con sus ramificaciones de hijos:
```bash
pstree -p | grep ejercicio2
```
O también podés buscarlo de forma interactiva con el comando `top`.

---

## Punto 3: Reemplazo de Imagen de Proceso (`execv`)
El proceso arranca ejecutando `ejercicio3`, pero a mitad de camino solicita al Kernel destruir su propio código en memoria RAM y reemplazarlo por el código compilado de `ejercicio3_exec`, manteniendo el mismo PID intacto.

**1. Compilación:**
Primero hay que compilar el programa que va a ser inyectado, y luego el programa principal:
```bash
gcc ejercicio3_exec.c -o ejercicio3_exec
gcc ejercicio3.c -o ejercicio3
```

**2. Ejecución:**
```bash
./ejercicio3
```

---

## Punto 4: Súper Combinación (`fork` + `execv` doble)
El programa padre bifurca (clona) su proceso original dos veces. Al Hijo 1 lo intercepta y le inyecta un nuevo código ejecutable. Al Hijo 2 hace lo mismo, inyectándole otro código ejecutable distinto. El padre se queda esperando al final a que ambos terminen sus nuevas tareas.

**1. Compilación:**
Hay que compilar los dos programitas inyectables y luego el padre / orquestador principal:
```bash
gcc ejercicio4_exec1.c -o ejercicio4_exec1
gcc ejercicio4_exec2.c -o ejercicio4_exec2
gcc ejercicio4.c -o ejercicio4
```

**2. Ejecución:**
```bash
./ejercicio4
```
