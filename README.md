# Invernadero Inteligente

Sistema IoT para el monitoreo de variables ambientales
en un invernadero mediante ESP32.

## Sensores

- HW-390 / 20210603: sensor capacitivo analógico de humedad del suelo.
- OKI3101: módulo de fotoresistencia LDR para medir luz ambiental.
- LM35: sensor sensor analógico de temperatura.

## Arquitectura

ESP32 → WiFi → FastAPI → Dashboard Web

## Variables monitoreadas

### Humedad del suelo

El sensor HW-390 proporciona una lectura analógica
que es convertida a un porcentaje de humedad después
de realizar la calibración física.

### Temperatura ambiental

El sensor LM35 proporciona una lectura analógica
que es convertida a grados celsius.

### Luz ambiental

El sensor OKI3101 proporciona una lectura analógica
que inicialmente se representa como un porcentaje
relativo de luz.

## Tecnologías

- ESP32
- Arduino IDE
- Python
- FastAPI
- HTML
- CSS
- JavaScript

## Instalación del backend

Desde la carpeta backend ejecutar:

```bash
pip install -r requirements.txt

y despúes python -m uvicorn main:app --host 0.0.0.0 --port 8000
