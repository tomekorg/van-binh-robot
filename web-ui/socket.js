export function initWebSocket() {
  if (process.env.DEVELOPMENT === "true") {
    console.warn("Running in development mode. Using mock socket.");
    return mockSocket();
  }

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

  return socket;

  function mockSocket() {
    return {
      send: (data) => console.log(data),
      onmessage: null,
      onclose: null,
      close: () => console.log("Mock socket closed"),
    };
  }
}