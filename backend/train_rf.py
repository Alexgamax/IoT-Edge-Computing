import numpy as np
from sklearn.ensemble import RandomForestClassifier
import joblib

np.random.seed(42)
n_samples = 1500

humedad = np.random.uniform(0, 100, n_samples)
luz = np.random.uniform(0, 100, n_samples)
temperatura = np.random.uniform(10, 50, n_samples)

X = np.column_stack((humedad, luz, temperatura))
y = []

for h, l, t in X:
    if t > 38.0 and h < 25.0:
        y.append("Riesgo de Incendio")
    elif h < 30.0:
        y.append("Riego Necesario")
    else:
        y.append("Operacion Normal")

y = np.array(y)

rf_model = RandomForestClassifier(n_estimators=50, random_state=42)
rf_model.fit(X, y)

joblib.dump(rf_model, "random_forest_model.pkl")
print("Modelo Random Forest entrenado y guardado como 'random_forest_model.pkl' exitosamente.")