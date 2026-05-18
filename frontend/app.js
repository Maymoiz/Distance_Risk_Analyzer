let port, reader;
let history = JSON.parse(localStorage.getItem("history") || "[]");
let alertThreshold = 50;
let smoothing = 0.3;

const alertSound = new Audio("https://actions.google.com/sounds/v1/alarms/beep_short.ogg");

/* ---------- LOADER ---------- */
window.onload = () => {
  setTimeout(() => {
    document.getElementById("loader").style.display = "none";
  }, 1200);
};

/* ---------- THEME ---------- */
document.getElementById("themeToggle").onclick = () =>
  document.body.classList.toggle("light");

/* ---------- SETTINGS MODAL ---------- */
document.getElementById("settingsBtn").onclick = () =>
  document.getElementById("settingsModal").style.display = "flex";

document.getElementById("closeSettings").onclick = () =>
  document.getElementById("settingsModal").style.display = "none";

document.getElementById("saveSettings").onclick = () => {
  alertThreshold = parseInt(document.getElementById("thresholdInput").value);
  smoothing = parseFloat(document.getElementById("smoothInput").value);
  document.getElementById("settingsModal").style.display = "none";
};

/* ---------- HISTORY DRAWER ---------- */
document.getElementById("historyBtn").onclick = () =>
  document.getElementById("historyDrawer").classList.toggle("open");

/* ---------- DEVICE STATUS ---------- */
function setStatus(status) {
  const badge = document.getElementById("deviceStatus");

  if (status === "online") {
    badge.className = "status-badge online";
    badge.textContent = "● Connected";
  } else if (status === "connecting") {
    badge.className = "status-badge connecting";
    badge.textContent = "● Connecting...";
  } else {
    badge.className = "status-badge offline";
    badge.textContent = "● Disconnected";
  }
}

/* ---------- CONNECT ---------- */
document.getElementById("connectBtn").onclick = async () => {
  if (!("serial" in navigator)) {
    alert("Web Serial not supported. Use Chrome/Edge.");
    return;
  }

  setStatus("connecting");

  port = await navigator.serial.requestPort();
  await port.open({ baudRate: 9600 });

  setStatus("online");
  logHistory("Connected");

  const decoder = new TextDecoderStream();
  port.readable.pipeTo(decoder.writable);
  reader = decoder.readable.getReader();

  let buffer = "";

  while (true) {
    const { value, done } = await reader.read();
    if (done) break;
    buffer += value;

    const lines = buffer.split("\n");
    buffer = lines.pop();

    for (const line of lines) {
      const parts = line.trim().split(",");
      if (parts.length < 4) continue;

      const dist = parseFloat(parts[1]);
      const pir = parts[2] === "1";
      const state = parts[3];

      document.getElementById("distanceVal").innerText = dist + " cm";
      document.getElementById("pirVal").innerText = pir ? "MOTION" : "STILL";
      document.getElementById("stateVal").innerText = state;

      updateChart(dist);

      if (dist < alertThreshold) alertSound.play();
    }
  }
};

/* ---------- HISTORY ---------- */
function logHistory(event) {
  history.push({ event, time: new Date().toLocaleString() });
  localStorage.setItem("history", JSON.stringify(history));

  const list = document.getElementById("historyList");
  list.innerHTML = history
    .map(h => `<li>${h.time} — ${h.event}</li>`)
    .join("");
}
