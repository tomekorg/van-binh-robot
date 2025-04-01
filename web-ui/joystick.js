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
