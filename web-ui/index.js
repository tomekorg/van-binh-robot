import "./style.css";
import nipplejs from "nipplejs";

document.getElementById("start").addEventListener("click", function () {
  console.log("klik");
});

let options = {
  zone: document.getElementById("joystick"),
  color: "#ff4081",
  position: { left: "50%", top: "50%" },
  mode: "dynamic",
};

let manager = nipplejs.create(options);
