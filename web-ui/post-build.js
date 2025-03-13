/* This script runs in post-build process (see npm "build" script).
 * It produces single html file out of production build, which is
 * easier to serve from ESP32.
 */
const fs = require("fs");
const inline = require("web-resource-inliner");

console.log("running resource inliner...");

inline.html(
  {
    fileContent: readFileSync("./dist/index.html"),
    relativeTo: "./dist",
  },
  (err, result) => {
    if (err) {
      throw err;
    }
    fs.writeFileSync("./dist/bundle.html", result);
    console.log("bundle is ready: dist/bundle.html");
  },
);

function readFileSync(file) {
  const contents = fs.readFileSync(file, "utf8");
  return process.platform === "win32"
    ? contents.replace(/\r\n/g, "\n")
    : contents;
}
