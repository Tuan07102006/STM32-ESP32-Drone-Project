#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

// Sử dụng R"rawliteral(...)rawliteral" để chứa toàn bộ HTML/CSS/JS
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Drone PID Tuning - Offline</title>
    <style>
        body { font-family: sans-serif; background: #121212; color: #e0e0e0; margin: 0; padding: 15px; }
        .grid { display: grid; grid-template-columns: 2fr 1fr; gap: 15px; }
        .panel { background: #1e1e1e; padding: 15px; border-radius: 8px; border: 1px solid #333; }
        h3 { margin-top: 0; color: #29b6f6; border-bottom: 1px solid #333; padding-bottom: 5px; }
        canvas { background: #000; width: 100%; height: 180px; border-radius: 4px; margin-bottom: 10px; }
        .data-card { background: #2a2a2a; padding: 10px; margin-bottom: 10px; border-radius: 5px; border-left: 4px solid #00e676; }
        .val { font-family: monospace; font-size: 1.2em; font-weight: bold; color: #00e676; }
        .btn { width: 100%; padding: 15px; font-weight: bold; border: none; border-radius: 5px; cursor: pointer; }
        .bg-red { background: #ff1744; color: white; }
        .bg-green { background: #00e676; color: black; }
    </style>
</head>
<body>
    <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 15px;">
        <h2>F450 PID TUNING STATION</h2>
        <div id="status" style="color: #ff1744;">WebSocket: DISCONNECTED</div>
    </div>

    <div class="grid">
        <div class="panel">
            <h3>📈 Biểu đồ thời gian thực (Offline)</h3>
            <canvas id="rollCanvas"></canvas>
            <canvas id="pitchCanvas"></canvas>
            <canvas id="yawCanvas"></canvas>
        </div>

        <div>
            <div class="panel" style="margin-bottom: 15px;">
                <button id="armBtn" class="btn bg-red" onclick="toggleArm()">DISARMED (Nhấn Y để ARM)</button>
            </div>
            <div class="panel">
                <h3>📡 Telemetry</h3>
                <div class="data-card">Pin: <span id="v" class="val">0.0</span> V</div>
                <div class="data-card">Ga (Thr): <span id="thr" class="val">1000</span></div>
                <div class="data-card">Roll (S/F): <span id="rs" class="val">0</span> / <span id="rf" class="val">0</span></div>
                <div class="data-card">Pitch (S/F): <span id="ps" class="val">0</span> / <span id="pf" class="val">0</span></div>
                <div id="joyInfo" style="font-size: 0.8em; color: #888; font-family: monospace;">Đợi tay cầm...</div>
            </div>
        </div>
    </div>

<script>
    class Scope {
        constructor(id, min, max, label) {
            this.canvas = document.getElementById(id);
            this.ctx = this.canvas.getContext('2d');
            this.min = min; this.max = max; this.label = label;
            this.dataSP = Array(100).fill(0);
            this.dataFB = Array(100).fill(0);
        }
        update(sp, fb) {
            this.dataSP.push(sp); this.dataSP.shift();
            this.dataFB.push(fb); this.dataFB.shift();
            this.draw();
        }
        draw() {
            let w = this.canvas.width; let h = this.canvas.height;
            this.ctx.clearRect(0,0,w,h);
            this.ctx.strokeStyle = '#333'; this.ctx.strokeRect(0,0,w,h);
            const getY = (v) => h - ((v - this.min) / (this.max - this.min)) * h;
            // Draw Zero Line
            this.ctx.beginPath(); this.ctx.strokeStyle = '#444'; this.ctx.moveTo(0, getY(0)); this.ctx.lineTo(w, getY(0)); this.ctx.stroke();
            // Draw Feedback (Blue)
            this.ctx.beginPath(); this.ctx.strokeStyle = '#29b6f6';
            for(let i=0; i<100; i++) this.ctx.lineTo((i/99)*w, getY(this.dataFB[i])); this.ctx.stroke();
            // Draw Setpoint (Green)
            this.ctx.beginPath(); this.ctx.strokeStyle = '#00e676';
            for(let i=0; i<100; i++) this.ctx.lineTo((i/99)*w, getY(this.dataSP[i])); this.ctx.stroke();
            this.ctx.fillStyle = '#fff'; this.ctx.fillText(this.label, 10, 20);
        }
    }

    const rollScope = new Scope('rollCanvas', -45, 45, "ROLL");
    const pitchScope = new Scope('pitchCanvas', -45, 45, "PITCH");
    const yawScope = new Scope('yawCanvas', 0, 360, "YAW");

    let ws = new WebSocket(`ws://${window.location.hostname}/ws`);
    let cmd = { arm: 0, thr: 1000, rs: 0, ps: 0, ys: 0 };

    ws.onopen = () => document.getElementById('status').style.color = '#00e676';
    ws.onclose = () => document.getElementById('status').style.color = '#ff1744';
    ws.onmessage = (e) => {
        let d = JSON.parse(e.data);
        document.getElementById('v').innerText = d.v.toFixed(2);
        document.getElementById('thr').innerText = d.thr;
        document.getElementById('rs').innerText = d.rs.toFixed(1); document.getElementById('rf').innerText = d.rf.toFixed(1);
        document.getElementById('ps').innerText = d.ps.toFixed(1); document.getElementById('pf').innerText = d.pf.toFixed(1);
        rollScope.update(d.rs, d.rf);
        pitchScope.update(d.ps, d.pf);
        yawScope.update(d.ys, d.yf);
    };

    function toggleArm() {
        cmd.arm = cmd.arm ? 0 : 1;
        let b = document.getElementById('armBtn');
        b.innerText = cmd.arm ? "ARMED - DANGER!" : "DISARMED (Nhấn Y)";
        b.className = cmd.arm ? "btn bg-green" : "btn bg-red";
    }

    setInterval(() => {
        let gp = navigator.getGamepads()[0];
        if(gp) {
            document.getElementById('joyInfo').innerText = "Xbox Connected";
            cmd.thr = Math.round(((-gp.axes[1] + 1) / 2) * 800 + 1100); // 1100-1900
            cmd.rs = gp.axes[2] * 30;
            cmd.ps = -gp.axes[3] * 30;
            if(gp.buttons[3].pressed) cmd.arm = 1; // Y button
            if(gp.buttons[0].pressed) cmd.arm = 0; // A button
            if(ws.readyState === 1) ws.send(JSON.stringify(cmd));
        }
    }, 50);
</script>
</body>
</html>
)rawliteral";

#endif // ĐÂY LÀ DÒNG QUAN TRỌNG NHẤT ĐỂ SỬA LÕooooo