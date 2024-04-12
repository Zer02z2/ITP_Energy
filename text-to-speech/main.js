import SamJs from 'sam-js';
import converter from 'number-to-words';

let timeNow = new Date(Date.now() - 4 * 60 * 60 * 1000).toLocaleString();
let months = ["January", "Feburary", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];

let [dateAndYear, time] = timeNow.split(", ");
let [month, date, year] = dateAndYear.split("/");
let [hour, min, sec, amOrpm] = time.split(/[: ]/);
console.log(month, date, year, hour, min, sec, amOrpm);

month = months[parseInt(month) - 1];
date = converter.toWordsOrdinal(parseInt(date));
let [year1, year2] = [year.slice(0, 2), year.slice(2, 4)];
console.log(year1, year2);
year1 = converter.toWords(parseInt(year1));
year2 = converter.toWords(parseInt(year2));
year = year1 + " " + year2;
dateAndYear = month + " " + date + ", " + year;

hour = converter.toWords(parseInt(hour));
min = converter.toWords(parseInt(min));
amOrpm = amOrpm.slice(0, 1) + "-" + amOrpm.slice(1, 2);
time = hour + ", " + min + " " + amOrpm;

let message = "Now it is " + time + ", " + dateAndYear + ". This is Wall li reporting. ";
console.log(message);

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
