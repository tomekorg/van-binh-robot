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

joystick.on('move', (evt, data) => {
  if (data.vector) {
    console.log(`X: ${data.vector.x}, Y: ${data.vector.y}`);

    const message = {
      type: "joystick",
      payload: {
        x: Math.round(data.vector.x * 1000),
        y: Math.round(data.vector.y * 1000),
      }
    }

    socket.send(JSON.stringify(message));
  }
});
