import "./style.css";
import nipplejs from "nipplejs";

// Create a WebSocket connection to the server
const socket = new WebSocket("ws://192.168.1.22/ws");

socket.onopen = function () {
  console.log("WebSocket connection established");
};

socket.onclose = function () {
  console.log("WebSocket connection closed");
};

socket.onerror = function (error) {
  console.error("WebSocket error:", error);
};

document.getElementById("start").addEventListener("click", function () {
  console.log("klik");
});

let options = {
  zone: document.getElementById("joystick"),
  color: "#ff4081",
  mode: "dynamic",
};

let joystick = nipplejs.create(options);

let lastVector = { x: 0, y: 0 }; // Store the last known joystick position
let intervalId = null; // Store the interval ID

joystick.on('start', () => {
  // Start sending events periodically when the joystick becomes active
  intervalId = setInterval(() => {
    const message = {
      type: "joystick",
      payload: {
        x: Math.round(lastVector.x * 1000),
        y: Math.round(lastVector.y * 1000),
      }
    };
    socket.send(JSON.stringify(message));
  }, 100); // Send every 100ms (adjust as needed)
});

joystick.on('move', (evt, data) => {
  if (data.vector) {
    lastVector = { x: data.vector.x, y: data.vector.y }; // Update the last known position
    console.log(`X: ${lastVector.x}, Y: ${lastVector.y}`);
  }
});

joystick.on('end', () => {
  // Stop sending events when the joystick is released
  clearInterval(intervalId);
  intervalId = null;
});
