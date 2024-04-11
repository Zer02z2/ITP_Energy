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

let serverIP = "io.zongzechen.com";
let endpoint = "/report";
const url = `https://${serverIP}${endpoint}`;

let endpoint2 = "/soundFile";
const url2 = `https://${serverIP}${endpoint2}`;

const socket = new WebSocket('wss:io.zongzechen.com/sensorValues');
socket.addEventListener('open', () => {
  console.log("Web socket opened successfully");
});
socket.addEventListener('message', handleSocketMessages);

function handleSocketMessages(e) {
  console.log(e.data);
  sendSoundFile(e.data);
}

testButton.addEventListener('click', () => {
  sendSoundFile(report);
});

getButton.addEventListener('click', () => {
  fetch(url)
    .then(result => result.text())
    .then((text) => {
      content.innerHTML = text + "<br>" + content.innerHTML;
    });
})

function sendSoundFile(report) {
  let buf8 = sam.buf8(report);
  console.log("sending");
  fetch(url2, {
    method: "POST",
    body: buf8,
    headers: {"Content-Type": "audio/wav"}
  });
}
