import SamJs from 'sam-js';

let report = "";

let opts = {
  debug: 1,
  pitch: 60,
  speed: 92,
  mouth: 190,
  throat: 190
};

let sam = new SamJs(opts);

let testButton = document.getElementById("test");
let getButton = document.getElementById("get");
let content = document.getElementById("content");

let serverIP = "142.93.244.227";
let port = 1880;
let endpoint = "/report";
const url = `http://${serverIP}:${port}${endpoint}`;

const socket = new WebSocket('ws://localhost:1880/sensorValues');
socket.addEventListener('open', () => {
  console.log("Web socket opened successfully");
});
socket.addEventListener('message', handleSocketMessages);

function handleSocketMessages(e) {
  report += e.data;
  console.log(e.data);

  if (e.data.substring(0, 3) == "CO2") {
    console.log(report);
    sam.speak(report);
    report = "";
  }
}

testButton.addEventListener('click', () => {
  sam.speak('Temperature is 25 degree celcius');
});

getButton.addEventListener('click', () => {
  fetch(url)
    .then(result => result.text())
    .then((text) => {
      content.innerHTML = text + "<br>" + content.innerHTML;
    })
})

