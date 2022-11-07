# dsp-3-2022-proyecto1-daniel-chris
dsp-3-2022-proyecto1-daniel-chris created by GitHub Classroom
# Dependencias
Cliente:
python 3.9 o superior
paquete sounddevice y NumPy para Python 3

Servidor:
Jack2 y la biblioteca estandar de C.



# Construcción y Ejecución

Para ejecutar el cliente (marcador), desde el directorio base ejecutar el script marker.py [duracion en ms] [ventana][opcional]. ej:

python marker.py 200 -w hann
python marker.py 350 -w triag
python marker.py 1050

Para el servidor ejecutar los siguientes comandos:
```
cd proj
make
./jack -c
```
