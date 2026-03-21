clc; % limpia la pantalla
clear; % borra el espacio de trabajo

% ejercicio 3
signal_freq=25;
signal_amp=25;
sampling_freq=2000;
sample_number = 500;
for i=1:sample_number
 signal(i) = (signal_amp*sin(2*pi*(i-1)*signal_freq/sampling_freq));
end
plot(signal);
plot(signal, 'o-b');
signal1 = signal;


signal_freq=50;
signal_amp=25;
sampling_freq=2000;
sample_number = 500;
for i=1:sample_number
 signal(i) = (signal_amp*sin(2*pi*(i-1)*signal_freq/sampling_freq));
end
plot(signal);
plot(signal, 'o-b');
signal2 = signal;

signal3 = signal1 + signal2;
plot(signal3, 'o-b');
