import "./style.css";
import { initJoystick } from "./joystick";
import { initWebSocket } from "./socket";

const socket = initWebSocket();
initJoystick(socket);
