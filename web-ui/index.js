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

let manager = nipplejs.create(options);
