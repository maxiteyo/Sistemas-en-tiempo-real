clc; % limpia la pantalla
clear; % borra el espacio de trabajo

% ejercicio 4
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

% --- PRIMERO: Espectro de la señal simple (signal1) ---
X = signal1;
L = length(X);
T = 1/sampling_freq;
t = (0:L-1)*T;
Y = fft(X); % calcula la FFT.
P2 = abs(Y/L);
P1 = P2(1:L/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = sampling_freq*(0:(L/2))/L;
plot(f,P1)
title('Amplitud en frecuencia - Señal 1 (25Hz)')
xlabel('f (Hz)')
ylabel('|P1(f)|')
xlim([0 500])

% ---------------------------------------------------------

% --- SEGUNDO: Espectro de la señal sumada (signal3) ---
X = signal3;
L = length(X);
T = 1/sampling_freq;
t = (0:L-1)*T;
Y = fft(X); % calcula la FFT.
P2 = abs(Y/L);
P1 = P2(1:L/2+1);
P1(2:end-1) = 2*P1(2:end-1);
f = sampling_freq*(0:(L/2))/L;
plot(f,P1)
title('Amplitud en frecuencia - Señal Sumada')
xlabel('f (Hz)')
ylabel('|P1(f)|')
xlim([0 500])



