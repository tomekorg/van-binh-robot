import "./style.css";
import nipplejs from "nipplejs";

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
  }
});
