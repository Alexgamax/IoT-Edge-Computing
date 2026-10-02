from fastapi import FastAPI
from datetime import datetime
from pydantic import BaseModel

app = FastAPI(
    title="Invernadero Inteligente",
    description="API para monitoreo de humedad del suelo y luz ambiental",
    version="1.0"
)


# =========================
# MODELO DE DATOS
# =========================

class Lectura(BaseModel):

    humedad_suelo: float
    luz: float


# =========================
# VARIABLE PARA ÚLTIMA LECTURA
# =========================

ultima_lectura = {
    "humedad_suelo": 0,
    "luz": 0,
    "timestamp": None
}


# =========================
# RECIBIR DATOS DE ESP32
# =========================

@app.post("/lecturas")
def recibir_lectura(lectura: Lectura):

    global ultima_lectura

    ultima_lectura = {
        "humedad_suelo": lectura.humedad_suelo,
        "luz": lectura.luz,
        "timestamp": datetime.now().isoformat()
    }

    print("Nueva lectura recibida:")
    print(ultima_lectura)

    return {
        "mensaje": "Lectura recibida correctamente",
        "datos": ultima_lectura
    }


# =========================
# OBTENER ÚLTIMA LECTURA
# =========================

@app.get("/lecturas")
def obtener_lectura():

    return ultima_lectura


# =========================
# ESTADO DEL SERVIDOR
# =========================

@app.get("/")
def inicio():

    return {
        "mensaje": "API del Invernadero Inteligente funcionando",
        "estado": "activo"
    }