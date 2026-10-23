const WS_URL = "ws://localhost:8000/ws/telemetria";

let socket;
let chart;

function inicializarGrafica() {
    const ctx = document.getElementById('chartTelemetria').getContext('2d');
    chart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                { label: 'Humedad Suelo (%)', borderColor: '#0dcaf0', data: [], fill: false, tension: 0.2 },
                { label: 'Luz (%)', borderColor: '#ffc107', data: [], fill: false, tension: 0.2 },
                { label: 'Temperatura (°C)', borderColor: '#dc3545', data: [], fill: false, tension: 0.2 }
            ]
        },
        options: {
            responsive: true,
            scales: {
                x: { display: true, title: { display: true, text: 'Hora' } },
                y: { beginAtZero: true, max: 100 }
            }
        }
    });
}

function conectarWebSocket() {
    socket = new WebSocket(WS_URL);

    socket.onopen = function() {
        console.log("WebSocket Conectado exitosamente.");
        document.getElementById("conexion").textContent = "Conectado al Servidor";
        document.getElementById("indicador").style.backgroundColor = "#32a852";
    };

    socket.onmessage = function(event) {
        const payload = JSON.parse(event.data);
        console.log("Lectura recibida:", payload);

        if (payload.data) {
            const hum = payload.data.humedad_suelo_pct;
            const luz = payload.data.luz_pct;
            const temp = payload.data.temperatura_c;

            document.getElementById("humedad").textContent = hum.toFixed(1);
            document.getElementById("luz").textContent = luz.toFixed(1);
            document.getElementById("temperatura").textContent = temp.toFixed(1);
            document.getElementById("latencia").textContent = payload.latencia_ms.toFixed(0);

            const badge = document.getElementById("badgeDiagnostico");
            const diag = payload.diagnostico_rf;
            badge.textContent = diag;

            if (diag === "Riesgo de Incendio") {
                badge.className = "badge bg-danger badge-status";
            } else if (diag === "Riego Necesario") {
                badge.className = "badge bg-warning text-dark badge-status";
            } else {
                badge.className = "badge bg-success badge-status";
            }

            const horaActual = new Date().toLocaleTimeString();
            document.getElementById("actualizacion").textContent = "Última act: " + horaActual;

            if (chart) {
                if (chart.data.labels.length > 20) {
                    chart.data.labels.shift();
                    chart.data.datasets.forEach(dataset => dataset.data.shift());
                }
                chart.data.labels.push(horaActual);
                chart.data.datasets[0].data.push(hum);
                chart.data.datasets[1].data.push(luz);
                chart.data.datasets[2].data.push(temp);
                chart.update();
            }
        }
    };

    socket.onclose = function() {
        console.warn("Conexión WebSocket cerrada. Reintentando en 3 segundos...");
        document.getElementById("conexion").textContent = "Sin Conexión";
        document.getElementById("indicador").style.backgroundColor = "#d9534f";
        setTimeout(conectarWebSocket, 3000);
    };

    socket.onerror = function(error) {
        console.error("Error WebSocket:", error);
    };
}

function enviarComando(estado) {
    if (socket && socket.readyState === WebSocket.OPEN) {
        const comando = {
            target: "ESP32_INVERNADERO_01",
            action: "ACTUADOR",
            value: estado
        };
        socket.send(JSON.stringify(comando));
        
        document.getElementById("estadoActuador").textContent = 
            "Comando enviado: " + (estado ? "ENCENDER (HIGH)" : "APAGAR (LOW)");
    } else {
        alert("No hay conexión con el servidor WebSocket.");
    }
}


window.onload = function() {
    inicializarGrafica();
    conectarWebSocket();
};