clc; % limpia la pantalla
clear; % borra el espacio de trabajo

% ejercicio 5
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

% carga de datos
array = signal3;
min_index = 1;
max_index = 100;
file_name = 'test_signal.h';
const_name = 'ELEMENTS';
array_name = ['short test_signal[' const_name ']={'];
% crea el archivo de cabecera
fid=fopen(file_name,'w');
fprintf(fid,'#define ');
fprintf(fid, const_name);
fprintf(fid, ' %d\n\n', max_index);
fprintf(fid, array_name);
fprintf(fid,'%d, ' , array(min_index:(max_index-1)));
fprintf(fid,'%d' , array(max_index));
fprintf(fid,'};\n');
fclose(fid);

