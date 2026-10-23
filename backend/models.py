from sqlalchemy import Column, Integer, Float, String, DateTime
from datetime import datetime
from database import Base

class Telemetria(Base):
    __tablename__ = "telemetria"

    id = Column(Integer, primary_key=True, index=True, autoincrement=True)
    sequence_id = Column(Integer, index=True)
    node_id = Column(String, index=True)
    humedad_suelo_pct = Column(Float)
    luz_pct = Column(Float)
    temperatura_c = Column(Float)
    clase_predicha_rf = Column(String)  # diagnostico Random Forest
    latencia_ms = Column(Float)        # t_recepcion - t_envio
    fecha_registro = Column(DateTime, default=datetime.utcnow)