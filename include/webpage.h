#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <Arduino.h>

// HTML Page stored in PROGMEM to save RAM
const char WEBPAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 PID Controller</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: 'Arial', sans-serif;
            background: linear-gradient(135deg, #e0e0e0 0%, #f5f5f5 100%);
            color: #333;
            padding: 20px;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .header {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            margin-bottom: 20px;
            text-align: center;
        }
        .header h1 {
            color: #555;
            font-size: 2em;
        }
        .header p {
            color: #666;
            margin-top: 10px;
        }
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-bottom: 20px;
        }
        .card {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
        }
        .card h3 {
            color: #555;
            margin-bottom: 15px;
            font-size: 1.2em;
        }
        .value {
            font-size: 2.5em;
            font-weight: bold;
            color: #333;
            margin: 10px 0;
        }
        .unit {
            font-size: 0.5em;
            color: #999;
        }
        .label {
            color: #666;
            font-size: 0.9em;
        }
        .chart-container {
            background: white;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.1);
            margin-bottom: 20px;
        }
        canvas {
            max-width: 100%;
        }
        .relay-status {
            display: flex;
            justify-content: space-around;
            margin-top: 15px;
        }
        .relay {
            text-align: center;
            padding: 10px;
            border-radius: 5px;
            flex: 1;
            margin: 0 5px;
        }
        .relay.on {
            background: #4caf50;
            color: white;
        }
        .relay.off {
            background: #f44336;
            color: white;
        }
        .status {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 5px;
        }
        .status.online {
            background: #4caf50;
            animation: pulse 2s infinite;
        }
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        @media (max-width: 768px) {
            .grid {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🌡️ ESP32 PID Temperature Controller</h1>
            <p><span class="status online"></span> System Status: <span id="status">Online</span></p>
        </div>

        <div class="grid">
            <div class="card">
                <h3>🎯 Setpoint (SP)</h3>
                <div class="value" id="setpoint">--<span class="unit">°C</span></div>
                <div class="label">Target Temperature</div>
            </div>
            <div class="card">
                <h3>🌡️ Process Value (PV)</h3>
                <div class="value" id="pv">--<span class="unit">°C</span></div>
                <div class="label">Current Temperature</div>
            </div>
            <div class="card">
                <h3>⚡ Output Power</h3>
                <div class="value" id="output">--<span class="unit">%</span></div>
                <div class="label">Heater Power</div>
            </div>
        </div>

        <div class="chart-container">
            <h3>📈 Temperature Trend</h3>
            <canvas id="tempChart"></canvas>
        </div>

        <div class="grid">
            <div class="card">
                <h3>⚙️ PID Parameters</h3>
                <div class="label">Kp (Proportional)</div>
                <div class="value" style="font-size:1.5em;" id="kp">--</div>
                <div class="label">Ki (Integral)</div>
                <div class="value" style="font-size:1.5em;" id="ki">--</div>
                <div class="label">Kd (Derivative)</div>
                <div class="value" style="font-size:1.5em;" id="kd">--</div>
            </div>
            <div class="card">
                <h3>🔌 Relay Status</h3>
                <div class="relay-status">
                    <div class="relay" id="relay1">
                        <div>Relay 1</div>
                        <div>OFF</div>
                    </div>
                    <div class="relay" id="relay2">
                        <div>Relay 2</div>
                        <div>OFF</div>
                    </div>
                    <div class="relay" id="relay3">
                        <div>Relay 3</div>
                        <div>OFF</div>
                    </div>
                </div>
            </div>
        </div>
    </div>

    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script>
        // Chart Configuration
        const ctx = document.getElementById('tempChart').getContext('2d');
        const maxDataPoints = 50;
        
        // Create gradient for Setpoint
        const gradientSP = ctx.createLinearGradient(0, 0, 0, 400);
        gradientSP.addColorStop(0, 'rgba(255, 99, 132, 0.6)');
        gradientSP.addColorStop(1, 'rgba(255, 99, 132, 0.0)');
        
        // Create gradient for PV
        const gradientPV = ctx.createLinearGradient(0, 0, 0, 400);
        gradientPV.addColorStop(0, 'rgba(54, 162, 235, 0.6)');
        gradientPV.addColorStop(1, 'rgba(54, 162, 235, 0.0)');
        
        const chart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Setpoint (SP)',
                    data: [],
                    borderColor: '#ff6384',
                    backgroundColor: gradientSP,
                    borderWidth: 3,
                    tension: 0.4,
                    fill: true,
                    pointRadius: 0,
                    pointHoverRadius: 5
                }, {
                    label: 'Process Value (PV)',
                    data: [],
                    borderColor: '#36a2eb',
                    backgroundColor: gradientPV,
                    borderWidth: 3,
                    tension: 0.4,
                    fill: true,
                    pointRadius: 0,
                    pointHoverRadius: 5
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: true,
                aspectRatio: 2,
                animation: {
                    duration: 0
                },
                scales: {
                    y: {
                        min: 0,
                        max: 140,
                        title: {
                            display: true,
                            text: 'Temperature (°C)'
                        },
                        ticks: {
                            stepSize: 20
                        }
                    },
                    x: {
                        title: {
                            display: true,
                            text: 'Time'
                        }
                    }
                },
                plugins: {
                    legend: {
                        display: true,
                        position: 'top'
                    }
                }
            }
        });

        // Update data from ESP32
        function updateData() {
            fetch('/data')
                .then(response => response.json())
                .then(data => {
                    // Update values
                    document.getElementById('setpoint').innerHTML = data.setpoint.toFixed(1) + '<span class="unit">°C</span>';
                    document.getElementById('pv').innerHTML = data.pv.toFixed(1) + '<span class="unit">°C</span>';
                    document.getElementById('output').innerHTML = data.output + '<span class="unit">%</span>';
                    document.getElementById('kp').textContent = data.kp.toFixed(1);
                    document.getElementById('ki').textContent = data.ki.toFixed(2);
                    document.getElementById('kd').textContent = data.kd.toFixed(2);

                    // Update relay status
                    updateRelay('relay1', data.relay1);
                    updateRelay('relay2', data.relay2);
                    updateRelay('relay3', data.relay3);

                    // Update chart
                    const now = new Date();
                    const timeStr = now.getHours().toString().padStart(2,'0') + ':' + 
                                   now.getMinutes().toString().padStart(2,'0') + ':' + 
                                   now.getSeconds().toString().padStart(2,'0');
                    
                    chart.data.labels.push(timeStr);
                    chart.data.datasets[0].data.push(data.setpoint);
                    chart.data.datasets[1].data.push(data.pv);

                    // Keep only last maxDataPoints
                    if (chart.data.labels.length > maxDataPoints) {
                        chart.data.labels.shift();
                        chart.data.datasets[0].data.shift();
                        chart.data.datasets[1].data.shift();
                    }

                    chart.update();
                })
                .catch(error => {
                    console.error('Error fetching data:', error);
                    document.getElementById('status').textContent = 'Offline';
                });
        }

        function updateRelay(id, state) {
            const relay = document.getElementById(id);
            if (state) {
                relay.className = 'relay on';
                relay.children[1].textContent = 'ON';
            } else {
                relay.className = 'relay off';
                relay.children[1].textContent = 'OFF';
            }
        }

        // Update every 1 second
        updateData();
        setInterval(updateData, 1000);
    </script>
</body>
</html>
)rawliteral";

#endif
