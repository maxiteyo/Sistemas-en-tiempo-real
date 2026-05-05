import sys
import threading
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

# =========================================================================
# CONFIGURACIÓN DEL GRAFICADOR
# =========================================================================
# Guardamos los últimos 100 valores para hacer el efecto de "ventana que avanza"
MAX_PUNTOS = 100

data_x = deque([0.0]*MAX_PUNTOS, maxlen=MAX_PUNTOS)
data_y = deque([0.0]*MAX_PUNTOS, maxlen=MAX_PUNTOS)
data_z = deque([0.0]*MAX_PUNTOS, maxlen=MAX_PUNTOS)

# Crear la figura y el eje
fig, ax = plt.subplots(figsize=(8, 5))
ax.set_title("Estabilidad de Plataforma - MPU6050 (Tiempo Real)")
ax.set_ylabel("Aceleración (Gravedades 'g')")
ax.set_xlabel("Muestras")

# Fijamos los límites Y en +/- 2g (que es la escala por defecto del sensor)
ax.set_ylim(-2.5, 2.5)
ax.grid(True, linestyle='--', alpha=0.6)

# Inicializar las tres líneas de la gráfica
line_x, = ax.plot(data_x, label='Eje X (Azul)', color='blue', linewidth=1.5)
line_y, = ax.plot(data_y, label='Eje Y (Naranja)', color='orange', linewidth=1.5)
line_z, = ax.plot(data_z, label='Eje Z (Verde)', color='green', linewidth=1.5)

ax.legend(loc='upper right')

# HILO DE LECTURA (Desacoplar recepción de datos y gráficos)
# =========================================================================
def lector_stdin():
    # Este bucle infinito lee el Pipe a máxima velocidad sin trabarse
    for linea in sys.stdin:
        try:
            valores = linea.strip().split(',')
            if len(valores) == 3:
                x, y, z = map(float, valores)
                data_x.append(x)
                data_y.append(y)
                data_z.append(z)
        except ValueError:
            pass

# Iniciar el hilo de lectura en segundo plano (daemon=True -> muere al cerrar el gráfico)
hilo = threading.Thread(target=lector_stdin, daemon=True)
hilo.start()

# =========================================================================
# FUNCIÓN DE ACTUALIZACIÓN (Se ejecuta en bucle por FuncAnimation)
# =========================================================================
def actualizar(frame):
    # Simplemente redibuja los datos que el hilo de lectura ya guardó
    line_x.set_ydata(data_x)
    line_y.set_ydata(data_y)
    line_z.set_ydata(data_z)
    return line_x, line_y, line_z

# =========================================================================
# MAIN
# =========================================================================
# interval=33 ms equivale a ~30 FPS visuales. Suficiente para el ojo humano.
# Evita que Matplotlib intente correr a 100Hz y colapse la Raspberry Pi.
ani = animation.FuncAnimation(fig, actualizar, interval=33, blit=True)

# Ajustar márgenes para que se vea lindo
plt.tight_layout()

# Mostrar la ventana (esto bloquea el script gráficamente hasta que la cerremos)
plt.show()