#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "Adafruit_SHTC3.h"
#include <Adafruit_DPS310.h>
#include <Wire.h>
#include "ScioSense_ENS160.h"

Adafruit_SHTC3 shtc3 = Adafruit_SHTC3(); // humidity 
Adafruit_DPS310 dps;
Adafruit_Sensor *dps_pressure = dps.getPressureSensor();
ScioSense_ENS160      ens160(ENS160_I2CADDR_1);

float count = 0;
int button = 2;
int lastReading = 0;

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
const int numOfTopics = 7;
const char pubTopic[] = "SolarWall_e/";
const String subTopics[] = {"count/", "humidity/", "temp/", "pressure/", "AQI/", "TVOC/", "CO2/"};

void setup() {
  Serial.begin(115200);

  // init SHTC3
  Serial.println("SHTC3 test");
  if (! shtc3.begin()) {
    Serial.println("Couldn't find SHTC3");
    while (1) delay(1);
  }
  Serial.println("Found SHTC3 sensor");

  // init DPS310
  Serial.println("DPS310");
  if (! dps.begin_I2C()) {
    Serial.println("Failed to find DPS");
    while (1) yield();
  }
  dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
  dps_pressure->printSensorDetails();
  Serial.println("DPS OK!");

  // init ENS160
  Serial.print("ENS160...");
  ens160.begin();
  Serial.println(ens160.available() ? "done." : "failed!");
  if (! ens160.available()) {
    Serial.println("Fail to start ENS260");
    while (1) delay(1);
  }
  Serial.print("\tRev: "); Serial.print(ens160.getMajorRev());
  Serial.print("."); Serial.print(ens160.getMinorRev());
  Serial.print("."); Serial.println(ens160.getBuild());
  Serial.print("\tStandard mode ");
  Serial.println(ens160.setMode(ENS160_OPMODE_STD) ? "done." : "failed!");

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    WiFi.begin(wifi_ssid, wifi_pass);
    delay(1000);
  }
  Serial.println("\nWiFi connected!");

  //give your device any name, to use for identification
  mqtt.setId("Zongze's Arduino");

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
}

const int sendInterval = 1000;
void loop() {

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    WiFi.begin(wifi_ssid, wifi_pass);
    delay(1000);
  }

  while (!mqtt.connect(broker, port)) {
    //error codes
    //  -1: credentials rejected
    //  -2: can't connect to broker
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqtt.connectError());
    mqtt.connect(broker, port);
    delay(200);
  }

  if (true || digitalRead(button) == HIGH && lastReading == LOW) {
    
    sensors_event_t humidity, temp, pressure_event;
    shtc3.getEvent(&humidity, &temp);
    dps_pressure->getEvent(&pressure_event);
    ens160.measure(true);
    ens160.measureRaw(true);

    float values[] = {count,humidity.relative_humidity, temp.temperature, pressure_event.pressure,
                      ens160.getAQI(), ens160.getTVOC(), ens160.geteCO2()};
    String units[] = {"", " g/kg", " C", " hPa", "", "ppb", "ppm"};

    Serial.println("Starting sending MQTT...");

    for (int i = 0; i < numOfTopics; i ++) {
      String topicName = pubTopic + subTopics[i];
      Serial.print("Topic: ");
      Serial.print(topicName);
      Serial.print(", Value: ");
      Serial.print(values[i]);
      Serial.println(units[i]);
      mqtt.beginMessage(topicName);
      mqtt.print(values[i]);
      mqtt.print(units[i]);
      mqtt.endMessage();
    }

    Serial.println("Complete sending");
    count += 1;
  }
  delay(60000);
  lastReading = digitalRead(button);
}
