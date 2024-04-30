#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "Adafruit_SHTC3.h"
#include <Adafruit_DPS310.h>
#include <Adafruit_AS7341.h>
#include <Wire.h>
#include "ScioSense_ENS160.h"
#include <Adafruit_SleepyDog.h>

Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();  // humidity
Adafruit_DPS310 dps;                      // pressure
Adafruit_Sensor *dps_pressure = dps.getPressureSensor();
ScioSense_ENS160 ens160(ENS160_I2CADDR_1);  //Gas sensor
Adafruit_AS7341 as7341;                     // light

float count = 0;
int sleepDuration = 5000;
int sleepGoal = 300000;

int sensorSwitch = 2;

WiFiClient wifi;
MqttClient mqtt(wifi);

//WiFi network info: ssid and password
const char wifi_ssid[] = "NewLabMember 2.4GHz Only";
const char wifi_pass[] = "!Welcome2NewLab!";

//MQTT broker info: url and port (1883 default for MQTT)
const char broker[] = "9.tcp.ngrok.io";
const int port = 24004;

//if needed: broker authentication credentials
const char mqtt_user[] = "energy";
const char mqtt_pass[] = "password";

//the topic this device will publish messages to
const int numOfTopics = 17;
const char pubTopic[] = "Sun-Watcher-01/";
const String subTopics[] = { "count/", "humidity/", "temp/", "pressure/", "AQI/", "TVOC/",
                            "CO2/", "light-415nm/", "light-445nm/", "light-480nm/",
                            "light-515nm/", "light-555nm/", "light-590nm/", "light-630nm/",
                            "light-680nm/", "light-clear/", "light-NIR/" };

void setup() {
  Watchdog.enable(8000);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(sensorSwitch, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(sensorSwitch, HIGH);
  Serial.begin(115200);

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
    delay(200);
  }
  Serial.println("MQTT connected.");
  Watchdog.reset();

  initSensors();
}

void loop() {

  if (WiFi.status() == WL_CONNECTED && mqtt.connect(broker, port)) {
    sensors_event_t humidity, temp, pressure_event;
    shtc3.getEvent(&humidity, &temp);
    dps_pressure->getEvent(&pressure_event);
    ens160.measure(true);
    ens160.measureRaw(true);
    uint16_t readings[12];
    as7341.readAllChannels(readings);

    float values[] = { count, humidity.relative_humidity, temp.temperature, pressure_event.pressure,
                       ens160.getAQI(), ens160.getTVOC(), ens160.geteCO2(), readings[0],readings[1],
                       readings[2], readings[3], readings[4], readings[5], readings[6], readings[7],
                       readings[8], readings[9], readings[10], readings[11]};
    String units[] = { "", " g/kg", " C", " hPa", "", " ppb", "ppm", "", "", "", "", "", "", "",
                      "", "", "", "", "" };

    Serial.println("Starting sending MQTT...");

    for (int i = 0; i < numOfTopics; i++) {
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
    digitalWrite(sensorSwitch, LOW);
    digitalWrite(LED_BUILTIN, LOW);

    Watchdog.disable();

    for (int i = 0; i < sleepGoal; i += sleepDuration) {
      // Serial.println("Going to sleep now");

      int sleepMS = Watchdog.sleep(4000);

      // ....... sleeping .......
    }

    Watchdog.enable(8000);

    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(sensorSwitch, HIGH);

    Serial.print("I'm awake now! I slept for ");
    Serial.print(sleepGoal);
    Serial.println(" milliseconds.");
    Serial.println();

    initSensors();
  }
}

void initSensors() {
  // init SHTC3
  Serial.println("SHTC3 test");
  while (!shtc3.begin()) {
    Serial.println("Couldn't find SHTC3");
    delay(100);
  }
  Serial.println("Found SHTC3 sensor");

  // init DPS310
  Serial.println("DPS310");
  while (!dps.begin_I2C()) {
    Serial.println("Failed to find DPS");
    delay(100);
  }
  dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
  dps_pressure->printSensorDetails();
  Serial.println("DPS OK!");

  // init ENS160
  Serial.print("ENS160...");
  ens160.begin();
  Serial.println(ens160.available() ? "done." : "failed!");
  while (!ens160.available()) {
    Serial.println("Fail to start ENS260");
    ens160.begin();
    delay(100);
  }
  Serial.print("\tRev: ");
  Serial.print(ens160.getMajorRev());
  Serial.print(".");
  Serial.print(ens160.getMinorRev());
  Serial.print(".");
  Serial.println(ens160.getBuild());
  Serial.print("\tStandard mode ");
  Serial.println(ens160.setMode(ENS160_OPMODE_STD) ? "done." : "failed!");

  // init AS7341
  if (!as7341.begin()) {
    Serial.println("Could not find AS7341");
    while (1) { delay(10); }
  }

  as7341.setATIME(100);
  as7341.setASTEP(999);
  as7341.setGain(AS7341_GAIN_256X);
}
