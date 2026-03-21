clc; % limpia la pantalla
clear; % borra el espacio de trabajo

% ejercicio 2
signal_freq=100;
signal_amp=100;
sampling_freq=8000;
sample_number = 2000;
for i=1:sample_number
 signal(i) = (signal_amp*sin(2*pi*(i-1)*signal_freq/sampling_freq));
end
plot(signal);
plot(signal, 'o-b');
signal1 = signal;
