import nipplejs from "nipplejs";

export function initJoystick(socket) {
  let joystickActive = false;
  let joystickData = { x: 0, y: 0 };
  let lastSentTime = 0;
  const throttleDelay = 100; // 100ms delay between messages

  const joystick = nipplejs.create({
    zone: document.getElementById("joystick"),
    color: "#ff4081",
    mode: "dynamic",
    size: 150, // Increased size of the joystick handle
  });

  joystick.on("start", () => {
    joystickActive = true;
    // Send a message immediately on start to switch to manual mode
    const message = {
      type: "joystick",
      payload: { x: 0, y: 0 }, // Initial position
    };
    socket.send(JSON.stringify(message));
  });

  joystick.on("move", (_, data) => {
    if (data.vector) {
      joystickActive = true;
      joystickData = {
        x: Math.round(data.vector.x * 1000),
        y: Math.round(data.vector.y * 1000),
      };
    }
  });

  joystick.on("end", () => {
    joystickActive = false;
    joystickData = { x: 0, y: 0 };
    // Send a final message to stop the motors
    const message = {
      type: "joystick",
      payload: joystickData,
    };
    socket.send(JSON.stringify(message));
  });

  function sendJoystickData() {
    const now = Date.now();
    if (joystickActive && now - lastSentTime >= throttleDelay) {
      const message = {
        type: "joystick",
        payload: joystickData,
      };
      socket.send(JSON.stringify(message));
      lastSentTime = now;
    }
    requestAnimationFrame(sendJoystickData);
  }

  sendJoystickData();
}