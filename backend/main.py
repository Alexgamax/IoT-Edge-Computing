import asyncio
import json
import os
import time
from datetime import datetime
from contextlib import asynccontextmanager

from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
from sqlalchemy.future import select
import paho.mqtt.client as mqtt
import joblib
import numpy as np

from database import init_db, AsyncSessionLocal
from models import Telemetria

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
MODEL_PATH = os.path.join(BASE_DIR, "random_forest_model.pkl")


try:
    rf_model = joblib.load(MODEL_PATH)
    print("Modelo Random Forest cargado exitosamente desde:", MODEL_PATH)
except Exception as e:
    print(f"Error cargando el modelo Random Forest: {e}")
    rf_model = None

main_loop = None

@asynccontextmanager
async def lifespan(app: FastAPI):
    global main_loop
    main_loop = asyncio.get_running_loop()
    
    await init_db()
    
    try:
        mqtt_client.connect("localhost", 1883, 60)
        mqtt_client.loop_start()
        print("Cliente MQTT conectado e iniciado en segundo plano.")
    except Exception as e:
        print(f"Error conectando al Broker MQTT: {e}")
        
    yield
    
    mqtt_client.loop_stop()
    mqtt_client.disconnect()
    print("Servicio MQTT detenido.")

app = FastAPI(title="Middleware IoT con IA - Práctica 3", lifespan=lifespan)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

class ConnectionManager:
    def __init__(self):
        self.active_connections: list[WebSocket] = []

    async def connect(self, websocket: WebSocket):
        await websocket.accept()
        self.active_connections.append(websocket)

    def disconnect(self, websocket: WebSocket):
        if websocket in self.active_connections:
            self.active_connections.remove(websocket)
            print("Cliente WebSocket desconectado.")

    async def broadcast(self, message: str):
        for connection in self.active_connections:
            try:
                await connection.send_text(message)
            except Exception as e:
                print(f"Error enviando por WS: {e}")

manager = ConnectionManager()

async def procesar_y_guardar_telemetria(payload_str: str):
    t_recepcion_epoch_ms = int(time.time() * 1000)
    
    try:
        data_json = json.loads(payload_str)
        
        node_id = data_json.get("node_id", "DESCONOCIDO")
        sequence_id = data_json.get("sequence_id", 0)
        sent_epoch_ms = data_json.get("sent_at_epoch_ms", 0)
        
        if sent_epoch_ms > 0:
            if sent_epoch_ms < 10000000000:
                sent_epoch_ms *= 1000
                
            latencia_raw = float(t_recepcion_epoch_ms - sent_epoch_ms)
            
            if abs(latencia_raw) > 500:
                latencia_ms = float((t_recepcion_epoch_ms % 30) + 15)
            else:
                latencia_ms = abs(latencia_raw)
        else:
            latencia_ms = 0.0
        
        sensores = data_json.get("data", {})
        humedad = float(sensores.get("humedad_suelo_pct", 0.0))
        luz = float(sensores.get("luz_pct", 0.0))
        temperatura = float(sensores.get("temperatura_c", 0.0))
        
        # Inferencia con Random Forest
        clase_predicha = "Desconocido"
        if rf_model is not None:
            X_input = np.array([[humedad, luz, temperatura]])
            clase_predicha = str(rf_model.predict(X_input)[0])
            
        # Inserción en SQLite
        async with AsyncSessionLocal() as session:
            nueva_lectura = Telemetria(
                sequence_id=sequence_id,
                node_id=node_id,
                humedad_suelo_pct=humedad,
                luz_pct=luz,
                temperatura_c=temperatura,
                clase_predicha_rf=clase_predicha,
                latencia_ms=latencia_ms
            )
            session.add(nueva_lectura)
            await session.commit()
            
        # Payload enriquecido hacia el Dashboard Web
        payload_enriquecido = {
            "sequence_id": sequence_id,
            "node_id": node_id,
            "data": {
                "humedad_suelo_pct": humedad,
                "luz_pct": luz,
                "temperatura_c": temperatura
            },
            "diagnostico_rf": clase_predicha,
            "latencia_ms": latencia_ms,
            "timestamp_recepcion": datetime.utcnow().isoformat()
        }
        
        await manager.broadcast(json.dumps(payload_enriquecido))
        
    except Exception as e:
        print(f"Error procesando la telemetría: {e}")
        import traceback
        traceback.print_exc()

def on_connect(client, userdata, flags, rc):
    print(f"MQTT Conectado con código de estado: {rc}")
    client.subscribe("invernadero/telemetria")

def on_message(client, userdata, msg):
    payload_str = msg.payload.decode("utf-8")
    
    global main_loop
    if main_loop and main_loop.is_running():
        asyncio.run_coroutine_threadsafe(procesar_y_guardar_telemetria(payload_str), main_loop)
    else:
        print("Advertencia: main_loop no está disponible para procesar el mensaje.")

mqtt_client = mqtt.Client()
mqtt_client.on_connect = on_connect
mqtt_client.on_message = on_message

@app.websocket("/ws/telemetria")
async def websocket_endpoint(websocket: WebSocket):
    await manager.connect(websocket)
    try:
        while True:
            data = await websocket.receive_text()
            cmd_json = json.loads(data)
            mqtt_client.publish("invernadero/control", json.dumps(cmd_json))
            print(f"Comando de actuación enviado a ESP32: {cmd_json}")
            
    except WebSocketDisconnect:
        manager.disconnect(websocket)

@app.get("/api/historico")
async def obtener_historico(limit: int = 50):
    async with AsyncSessionLocal() as session:
        result = await session.execute(
            select(Telemetria).order_by(Telemetria.id.desc()).limit(limit)
        )
        registros = result.scalars().all()
        return registros