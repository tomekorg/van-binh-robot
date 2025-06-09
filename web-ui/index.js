import "./style.css";
import { initJoystick } from "./joystick";
import { initWebSocket } from "./socket";

const logsContainer = document.getElementById("logs-container");
const startButton = document.getElementById("start");
const calibrateButton = document.getElementById("calibrate");
const sensorsContainer = document.getElementById("sensors-container");
const autoStatsContainer = document.getElementById("auto-stats");
const kpInput = document.getElementById("kp");
const kiInput = document.getElementById("ki");
const kdInput = document.getElementById("kd");
const baseSpeedInput = document.getElementById("baseSpeed");
const maxMotorSpeedInput = document.getElementById("maxMotorSpeed");
const minAutoSpeedInput = document.getElementById("minAutoSpeed");
const ledcFrequencyInput = document.getElementById("ledcFrequency");
const updateParamsButton = document.getElementById("update-params");

// New input elements
const integralLimitInput = document.getElementById("integralLimit");
const turnSensitivityInput = document.getElementById("turnSensitivity");
const sharpTurnThresholdInput = document.getElementById("sharpTurnThreshold");
const sharpTurnKpInput = document.getElementById("sharpTurnKp");

const SENSOR_COUNT = 8;
const maxLogMessages = 200;

// Initialize sensor display
for (let i = 0; i < SENSOR_COUNT; i++) {
  const sensorWrapper = document.createElement("div");
  sensorWrapper.className = "sensor-wrapper";

  const sensorValue = document.createElement("div");
  sensorValue.className = "sensor-value";
  sensorValue.id = `sensor-value-${i}`;
  sensorValue.textContent = "0";

  const sensorLabel = document.createElement("div");
  sensorLabel.className = "sensor-label";
  sensorLabel.textContent = i;

  sensorWrapper.appendChild(sensorValue);
  sensorWrapper.appendChild(sensorLabel);
  sensorsContainer.appendChild(sensorWrapper);
}

function updateSensorsUI(values) {
  if (!values || values.length > SENSOR_COUNT) return;
  values.forEach((value, index) => {
    const sensorValue = document.getElementById(`sensor-value-${index}`);
    if (sensorValue) {
      sensorValue.textContent = value;
    }
  });
}

function updateAutoStatsUI(data) {
  if (!autoStatsContainer) return;
  document.getElementById("pos-val").textContent = data.position;
  document.getElementById("err-val").textContent = data.error;
  document.getElementById("corr-val").textContent = data.correction;
  document.getElementById("ls-val").textContent = data.leftSpeed;
  document.getElementById("rs-val").textContent = data.rightSpeed;
}

function addLogMessage(message) {
  if (!logsContainer) return;

  const logElement = document.createElement("div");
  logElement.className = "log-message";
  logElement.textContent = message.payload;

  const content = message.payload.trim();
  if (content.startsWith("E (")) logElement.classList.add("log-error");
  else if (content.startsWith("W (")) logElement.classList.add("log-warn");
  else if (content.startsWith("I (")) logElement.classList.add("log-info");
  else if (content.startsWith("D (")) logElement.classList.add("log-debug");
  else if (content.startsWith("V (")) logElement.classList.add("log-verbose");

  logsContainer.prepend(logElement);

  while (logsContainer.children.length > maxLogMessages) {
    logsContainer.removeChild(logsContainer.lastChild);
  }
}

function handleWebSocketMessage(event) {
  try {
    const message = JSON.parse(event.data);

    if (message.type === "log" && message.payload) {
      addLogMessage(message);
    } else if (message.type === "sensors" && message.payload) {
      autoStatsContainer.classList.add("hidden");
      updateSensorsUI(message.payload);
    } else if (message.type === "auto_state" && message.payload) {
      autoStatsContainer.classList.remove("hidden");
      updateSensorsUI(message.payload.sensors);
      updateAutoStatsUI(message.payload);
    } else if (message.type === "pid_state" && message.payload) {
      kpInput.value = message.payload.kp;
      kiInput.value = message.payload.ki;
      kdInput.value = message.payload.kd;
      if (message.payload.baseSpeed) baseSpeedInput.value = message.payload.baseSpeed;
      if (message.payload.maxMotorSpeed) maxMotorSpeedInput.value = message.payload.maxMotorSpeed;
      if (message.payload.minAutoSpeed) minAutoSpeedInput.value = message.payload.minAutoSpeed;
      if (message.payload.ledcFrequency) ledcFrequencyInput.value = message.payload.ledcFrequency;
      // Update new inputs
      if (message.payload.integralLimit) integralLimitInput.value = message.payload.integralLimit;
      if (message.payload.turnSensitivity) turnSensitivityInput.value = message.payload.turnSensitivity;
      if (message.payload.sharpTurnErrorThreshold) sharpTurnThresholdInput.value = message.payload.sharpTurnErrorThreshold;
      if (message.payload.sharpTurnKp) sharpTurnKpInput.value = message.payload.sharpTurnKp;
    } else {
      console.log("Received unhandled message:", message);
    }
  } catch (error) {
    console.error("Failed to parse WebSocket message:", error);
    addLogMessage({ type: "log", payload: event.data });
  }
}

const socket = initWebSocket();
socket.onmessage = handleWebSocketMessage;

// Send initial PID values once connected
socket.onopen = () => {
  console.log("WebSocket connection established. ESP32 will send initial state.");
};

initJoystick(socket);

startButton.addEventListener("click", () => {
  const message = {
    type: "start_auto",
  };
  socket.send(JSON.stringify(message));
  console.log("Start button clicked, switching to AUTO mode.");
});

calibrateButton.addEventListener("click", () => {
  const message = { type: "calibrate" };
  socket.send(JSON.stringify(message));
  console.log("Sent calibration request.");
});

updateParamsButton.addEventListener("click", () => {
  const message = {
    type: "update_params",
    payload: {
      kp: parseFloat(kpInput.value),
      ki: parseFloat(kiInput.value),
      kd: parseFloat(kdInput.value),
      baseSpeed: parseInt(baseSpeedInput.value, 10),
      maxMotorSpeed: parseInt(maxMotorSpeedInput.value, 10),
      minAutoSpeed: parseInt(minAutoSpeedInput.value, 10),
      ledcFrequency: parseInt(ledcFrequencyInput.value, 10),
      // Add new parameter values
      integralLimit: parseFloat(integralLimitInput.value),
      turnSensitivity: parseFloat(turnSensitivityInput.value),
      sharpTurnErrorThreshold: parseInt(sharpTurnThresholdInput.value, 10),
      sharpTurnKp: parseFloat(sharpTurnKpInput.value),
    },
  };
  socket.send(JSON.stringify(message));
  console.log("Sent params update:", message.payload);
});