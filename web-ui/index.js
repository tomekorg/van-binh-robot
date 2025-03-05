import nipplejs from "nipplejs";

document.getElementById("start").addEventListener("click", function () {
  console.log("klik");
});

let options = {
  zone: document.getElementById("joystick"),
  color: "red",
  mode: "static",
};

let manager = nipplejs.create(options);
