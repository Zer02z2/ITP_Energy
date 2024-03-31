#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>

WiFiClient wifi;
MqttClient mqtt(wifi);

//WiFi network info: ssid and password
const char wifi_ssid[] = "sandbox370";
const char wifi_pass[] = "+s0a+s03!2gether?";

//MQTT broker info: url and port (1883 default for MQTT)
const char broker[] = "9.tcp.ngrok.io";
const int  port = 24004;

//if needed: broker authentication credentials
const char mqtt_user[] = "energy";
const char mqtt_pass[] = "password";

//the topic this device will publish messages to
const char subTopic[] = "SolarWall_e/#";

//the maximum size of incoming message 
//ensure messages won't be longer than this!
const int bufferLength = 32;

void setup() {
  Serial.begin(9600);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    WiFi.begin(wifi_ssid, wifi_pass);
    delay(1000);
  }
  Serial.println("\nWiFi connected!");

  //give your device any name, to use for identification
  mqtt.setId("Henrique's Arduino"); 

  //set mqtt credentials, if needed
  mqtt.setUsernamePassword(mqtt_user, mqtt_pass);

  while (!mqtt.connect(broker, port)) {
    //error codes
    //  -1: credentials rejected
    //  -2: can't connect to broker
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqtt.connectError());
    mqtt.connect(broker, port);
    delay(200);
  } 
  
  Serial.println("MQTT connected.");
  mqtt.subscribe(subTopic);
}

void loop() {

  //run this function regularly to prevent 
  //your mqtt connection from timing out!
  mqtt.poll();

  if (mqtt.available()) {
    //Serial.print("Message received on topic: ");

    String topicName = mqtt.messageTopic();
    int valueStart = sizeof(subTopic);
    int valueEnd = topicName.indexOf("/", valueStart);
    topicName = topicName.substring(valueStart - 2, valueEnd);
    Serial.print(topicName);
    Serial.print(": ");
    //Serial.println(mqtt.messageTopic());

    //the raw message text (not parsed into a number)
    char messageBuffer[bufferLength];
    mqtt.readBytes(messageBuffer, bufferLength);

    //the message parsed into a usable float:
    //float messageFloat = atof(messageBuffer);

    //do something with message values here:
    //if (messageFloat > 500) {} //etc...

    Serial.println(messageBuffer);
    //Serial.println(atof(messageBuffer));
 
    Serial.println();
  }
  
}
