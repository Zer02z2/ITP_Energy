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

const socket = new WebSocket('ws://localhost/sensorValues');
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

document.getElementById("test").addEventListener('click', () => {
  sam.speak('Temperature is 25 degree celcius');
});

