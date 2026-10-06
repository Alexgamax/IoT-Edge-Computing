// IP
const API_URL = "http://192.168.100.17:8000";

async function obtenerDatos() {
    try {
        const respuesta = await fetch(`${API_URL}/lecturas`);

        if (!respuesta.ok) {
            throw new Error("No se pudieron obtener los datos");
        }

        const datos = await respuesta.json();

        if (datos.humedad_suelo !== undefined) {
            document.getElementById("humedad").textContent = datos.humedad_suelo.toFixed(1);
            document.getElementById("humedad2").textContent = datos.humedad_suelo.toFixed(1);
        }

        if (datos.luz !== undefined) {
            document.getElementById("luz").textContent = datos.luz.toFixed(1);
            document.getElementById("luz2").textContent = datos.luz.toFixed(1);
        }

        if (datos.temperatura !== undefined) {
            document.getElementById("temperatura").textContent = datos.temperatura.toFixed(1);
            document.getElementById("temperatura2").textContent = datos.temperatura.toFixed(1);
        }

        if (datos.timestamp) {
            const fecha = new Date(datos.timestamp);
            document.getElementById("hora").textContent = fecha.toLocaleTimeString();
            document.getElementById("actualizacion").textContent =
                "Última actualización: " + fecha.toLocaleTimeString();
        }

        document.getElementById("conexion").textContent = "ESP32 conectada";
        document.querySelector(".indicador").style.background = "#32a852";

    } catch (error) {
        console.error("Error al consultar API:", error);

        document.getElementById("conexion").textContent = "Sin conexión con el servidor";
        document.querySelector(".indicador").style.background = "#d9534f";
    }
}

obtenerDatos();

setInterval(obtenerDatos, 5000);