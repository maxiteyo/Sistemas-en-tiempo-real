clc; % limpia la pantalla
clear; % borra el espacio de trabajo

fs = 1000;  % frecuencia de muestreo
t = 0:1/fs:1;  % vector de tiempo
f = 5;  % frecuencia de la se#al
x = sin(2 * pi * f * t);  % se#al sinusoidal
bits = 7;  % bits de cuantización
levels = 2^bits;  % niveles de cuantización
x_quantized = round((x + 1) * (levels/2 - 1)) / (levels/2 - 1) - 1;
plot(t, x, 'b', 'LineWidth', 1);
hold on;
plot(t, x_quantized, 'r', 'LineWidth', 1.5);
xlabel('Tiempo [s]');
ylabel('Amplitud');
title(['Señal Cuantizada a ', num2str(bits), ' bits']);
legend('Original', 'Cuantizada');
grid on;
