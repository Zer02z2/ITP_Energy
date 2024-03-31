/*
  Dweet.io GET client for ArduinoHttpClient library
  Connects to dweet.io once every ten seconds,
  sends a GET request and a request body. Uses SSL

  Shows how to use Strings to assemble path and parse content
  from response. dweet.io expects:
  https://dweet.io/get/latest/dweet/for/thingName

  For more on dweet.io, see https://dweet.io/play/

  created 15 Feb 2016 updated 22 Jan 2019 by Tom Igoe 
  updated by Yeseul Song for Intangible in 2024

  this example is in the public domain
*/
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
/////// Wifi Settings ///////
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

String valueTypes[3] = {"\"count\":", "\"humidity\":", "\"temp\":"};
int results[] = {0, 0, 0};

const char serverAddress[] = "dweet.io";  // server address
int port = 80;
String dweetName = "zongzehenrique"; // use your own thing name here

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;


void setup() {
  pinMode(7, OUTPUT); digitalWrite(7, LOW);
  Serial1.begin(19200);
  
  Serial.begin(9600);
  while (!Serial);
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);     // print the network name (SSID);

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
  // assemble the path for the GET message:
  String path = "/get/latest/dweet/for/" + dweetName;

  // send the GET request
  Serial.println("making GET request");
  client.get(path);

  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

  /*
    Typical response is:
    {"this":"succeeded",
    "by":"getting",
    "the":"dweets",
    "with":[{"thing":"my-thing-name",
      "created":"2016-02-16T05:10:36.589Z",
      "content":{"sensorValue":456}}]}

    You want "content": numberValue
  */
  // now parse the response looking for "content":
  int labelStart = response.indexOf("content\":");
  // find the first { after "content":
  int contentStart = response.indexOf("{", labelStart);
  // find the following } and get what's between the braces:
  int contentEnd = response.indexOf("}", labelStart);
  String content = response.substring(contentStart + 1, contentEnd);
  Serial.println(content);

  // now get the value after the colon, and convert to an int:
  for (int i = 0; i < 3; i ++) {
    int valueStart = content.indexOf(valueTypes[i]);
    String valueString;
    if (i < 2) {
      int valueEnd = content.indexOf(",", valueStart);
      valueString = content.substring(valueStart, valueEnd);
    }
    else {
      valueString = content.substring(valueStart);
    }
    int numberStart = valueString.indexOf(":");
    int value = valueString.substring(numberStart + 1).toInt();
    results[i] = value;
    Serial.println(valueString);
    Serial.println(results[i]);
  }

  //printer.setFont('A');
  //printer.println(valueString);
  Serial.println("Wait ten seconds\n");
  
  delay(1000);
}
