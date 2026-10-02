// ======================================
// CONFIGURACIÓN
// ======================================

// Cambiar por la IP de la computadora
// donde se está ejecutando FastAPI.

const API_URL = "http://192.168.1.100:8000";


// ======================================
// OBTENER DATOS
// ======================================

async function obtenerDatos() {

    try {

        const respuesta = await fetch(
            `${API_URL}/lecturas`
        );

        if (!respuesta.ok) {

            throw new Error(
                "No se pudieron obtener los datos"
            );

        }

        const datos = await respuesta.json();


        // ==============================
        // ACTUALIZAR HUMEDAD
        // ==============================

        document.getElementById("humedad")
            .textContent =
            datos.humedad_suelo.toFixed(1);

        document.getElementById("humedad2")
            .textContent =
            datos.humedad_suelo.toFixed(1);


        // ==============================
        // ACTUALIZAR LUZ
        // ==============================

        document.getElementById("luz")
            .textContent =
            datos.luz.toFixed(1);

        document.getElementById("luz2")
            .textContent =
            datos.luz.toFixed(1);


        // ==============================
        // ACTUALIZAR HORA
        // ==============================

        if (datos.timestamp) {

            const fecha =
                new Date(datos.timestamp);

            document.getElementById("hora")
                .textContent =
                fecha.toLocaleTimeString();

            document.getElementById("actualizacion")
                .textContent =
                "Última actualización: " +
                fecha.toLocaleTimeString();
        }


        // ==============================
        // ESTADO CONEXIÓN
        // ==============================

        document.getElementById("conexion")
            .textContent =
            "ESP32 conectada";

        document.querySelector(".indicador")
            .style.background =
            "#32a852";


    } catch (error) {

        console.error(error);

        document.getElementById("conexion")
            .textContent =
            "Sin conexión con el servidor";

        document.querySelector(".indicador")
            .style.background =
            "#d9534f";
    }

}


// ======================================
// EJECUTAR
// ======================================

obtenerDatos();


// Actualizar cada 5 segundos

setInterval(
    obtenerDatos,
    5000
);