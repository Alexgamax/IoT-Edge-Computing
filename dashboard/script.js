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
                { 
                    label: 'Humedad Suelo (%)', 
                    borderColor: '#0dcaf0', 
                    backgroundColor: 'rgba(13, 202, 240, 0.1)',
                    data: [], 
                    fill: false, 
                    tension: 0.2,
                    borderWidth: 2,
                    pointRadius: 2,
                    yAxisID: 'y' 
                },
                { 
                    label: 'Luz (%)', 
                    borderColor: '#ffc107', 
                    backgroundColor: 'rgba(255, 193, 7, 0.1)',
                    data: [], 
                    fill: false, 
                    tension: 0.2,
                    borderWidth: 2,
                    pointRadius: 2,
                    yAxisID: 'y'
                },
                { 
                    label: 'Temperatura (°C)', 
                    borderColor: '#dc3545', 
                    backgroundColor: 'rgba(220, 53, 69, 0.1)',
                    data: [], 
                    fill: false, 
                    tension: 0.2,
                    borderWidth: 2,
                    pointRadius: 2,
                    yAxisID: 'y'
                },
                { 
                    label: 'Latencia Red (ms)', 
                    borderColor: '#6f42c1', 
                    backgroundColor: 'rgba(111, 66, 193, 0.1)',
                    data: [], 
                    fill: false, 
                    tension: 0.2,
                    borderWidth: 2,
                    borderDash: [5, 5], 
                    pointRadius: 2,
                    yAxisID: 'y1' 
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            animation: false,
            interaction: {
                mode: 'index',
                intersect: false,
            },
            scales: {
                x: { 
                    display: true, 
                    title: { display: true, text: 'Tiempo (Hora)' } 
                },
                y: { 
                    type: 'linear',
                    display: true,
                    position: 'left',
                    beginAtZero: true,
                    max: 100,
                    title: { display: true, text: 'Sensores (% / °C)' }
                },
                y1: { 
                    type: 'linear',
                    display: true,
                    position: 'right',
                    beginAtZero: true,
                    suggestedMax: 100,
                    title: { display: true, text: 'Latencia Red (ms)' },
                    grid: {
                        drawOnChartArea: false 
                    }
                }
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

        if (payload.data) {
            const hum = parseFloat(payload.data.humedad_suelo_pct);
            const luz = parseFloat(payload.data.luz_pct);
            const temp = parseFloat(payload.data.temperatura_c);
            const lat = parseFloat(payload.latencia_ms);

            document.getElementById("humedad").textContent = hum.toFixed(1);
            document.getElementById("luz").textContent = luz.toFixed(1);
            document.getElementById("temperatura").textContent = temp.toFixed(1);
            document.getElementById("latencia").textContent = lat.toFixed(0);

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

            const elemDetalles = document.getElementById("detallesIaa");
            if (elemDetalles) {
                elemDetalles.textContent = `Vector input: [H: ${hum.toFixed(1)}%, L: ${luz.toFixed(1)}%, T: ${temp.toFixed(1)}°C]`;
            }

            const horaActual = new Date().toLocaleTimeString();
            document.getElementById("actualizacion").textContent = "Última act: " + horaActual;

            if (chart) {
                if (chart.data.labels.length > 20) {
                    chart.data.labels.shift();
                    chart.data.datasets[0].data.shift();
                    chart.data.datasets[1].data.shift();
                    chart.data.datasets[2].data.shift();
                    chart.data.datasets[3].data.shift(); 
                }
                
                chart.data.labels.push(horaActual);
                chart.data.datasets[0].data.push(hum);  
                chart.data.datasets[1].data.push(luz);  
                chart.data.datasets[2].data.push(temp); 
                chart.data.datasets[3].data.push(lat);  
                
                chart.update('none');
            }
        }
    };

    socket.onclose = function() {
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