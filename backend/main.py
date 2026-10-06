from fastapi import FastAPI
from datetime import datetime
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel

app = FastAPI(
    title="Invernadero Inteligente",
    description="API para monitoreo de humedad del suelo y luz ambiental",
    version="1.0"
)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

class Lectura(BaseModel):
    humedad_suelo: float
    luz: float
    temperatura: float

ultima_lectura = {
    "humedad_suelo": 0,
    "luz": 0,
    "temperatura": 0,
    "timestamp": None
}

@app.post("/lecturas")
def recibir_lectura(lectura: Lectura):

    global ultima_lectura

    ultima_lectura = {
        "humedad_suelo": lectura.humedad_suelo,
        "luz": lectura.luz,
        "temperatura": lectura.temperatura,
        "timestamp": datetime.now().isoformat()
    }

    print("Nueva lectura recibida:")
    print(ultima_lectura)

    return {
        "mensaje": "Lectura recibida correctamente",
        "datos": ultima_lectura
    }

@app.get("/lecturas")
def obtener_lectura():

    return ultima_lectura

@app.get("/")
def inicio():

    return {
        "mensaje": "API del Invernadero Inteligente funcionando",
        "estado": "activo"
    }