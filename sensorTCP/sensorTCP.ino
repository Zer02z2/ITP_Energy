#include <WiFiNINA.h>

WiFiClient client;

int count = 0;

const char server[] = "10.23.10.19"; //local IP address of receiver device goes here
const int portNum = 2222; //desired port # goes here. Make sure the receiver is listening on the same port!

//be sure to remove WiFi network details before uploading this code!
const char WIFI_SSID[] = "sandbox370"; //WiFi network name goes here
const char WIFI_PASS[] = "+s0a+s03!2gether?"; //WiFi password goes here

void setup() {
  Serial.begin(9600);
  //retry connection until WiFi is connected successfully

  //init wifi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting to connect to SSID: ");
    // Attempt connection to WPA/WPA2 network.
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    delay(3000);
  }
  Serial.println("connected!");
}

void loop() {
  //connect to client if disconnected, or send TCP message if conected
  if (!client.connected()) {
    Serial.println("connecting");
    client.connect(server, portNum);
    delay(1000);
    return;

  } else {
    //add something more interesting here
    Serial.println("sending TCP message");
    client.println(count);
    // client.println(data);
    count += 1;
    delay(1000);
  }
}

