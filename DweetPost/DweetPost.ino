/*
  Dweet.io POST client for ArduinoHttpClient library
  Connects to dweet.io once every ten seconds,
  sends a POST request and a request body.

  Shows how to use Strings to assemble path and body

  created 15 Feb 2016 modified 22 Jan 2019 by Tom Igoe
  updated in 2024 by Yeseul Song for Intangible

  this example is in the public domain
*/
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include "Adafruit_SHTC3.h"

#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
int count = 0;
int button = 2;
int lastReading = 0;

Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();


/////// Wifi Settings ///////
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

const char serverAddress[] = "dweet.io";  // server address
int port = 80;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

void setup() {
  Serial.begin(9600);
  pinMode(button, INPUT);
  // while(!Serial);

  Serial.println("SHTC3 test");
  if (! shtc3.begin()) {
    Serial.println("Couldn't find SHTC3");
    while (1) delay(1);
  }
  Serial.println("Found SHTC3 sensor");

  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

void loop() {
  // assemble the path for the POST message:
  if (digitalRead(button) == HIGH && lastReading == LOW) {
    
    sensors_event_t humidity, temp;
    shtc3.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data

    String dweetName = "zongzehenrique";
    String path = "/dweet/for/" + dweetName;
    String contentType = "application/json";

    // assemble the body of the POST message:
    String postData = "{\"count\":value1,\"humidity\":value2,\"temp\":value3}";
    postData.replace("value1", String(count));
    postData.replace("value2", String(humidity.relative_humidity));
    postData.replace("value3", String(temp.temperature));

    Serial.print("making POST request: ");
    Serial.println(postData);

    // send the POST request
    client.post(path, contentType, postData);

    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);

    count += 1;
  }
  lastReading = digitalRead(button);
  
}
