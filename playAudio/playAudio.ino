#include <WiFiNINA.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include "arduino_secrets.h"

// define the pins used
//#define CLK 13       // SPI Clock, shared with SD card
//#define MISO 12      // Input data, from VS1053/SD card
//#define MOSI 11      // Output data, to VS1053/SD card
// Connect CLK, MISO and MOSI to hardware SPI pins.
// See http://arduino.cc/en/Reference/SPI "Connections"

// These are the pins used for the breakout example
#define BREAKOUT_RESET 9  // VS1053 reset pin (output)
#define BREAKOUT_CS 10    // VS1053 chip select pin (output)
#define BREAKOUT_DCS 8    // VS1053 Data/command select pin (output)
// These are the pins used for the music maker shield
#define SHIELD_RESET -1  // VS1053 reset pin (unused!)
#define SHIELD_CS 7      // VS1053 chip select pin (output)
#define SHIELD_DCS 6     // VS1053 Data/command select pin (output)

// These are common pins between breakout and shield
#define CARDCS 4  // Card chip select pin
// DREQ should be an Int pin, see http://arduino.cc/en/Reference/attachInterrupt
#define DREQ 3  // VS1053 Data request, ideally an Interrupt pin

File myFile;
String getString = "";
int state = 0;
long requestTime = 0;
long playStartTime = 0;

/////// WiFi Settings ///////
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

char serverAddress[] = "io.zongzechen.com";  // server IP address
int port = 443;                              //HTTP default is 80, or use 1880 for node-red

WiFiSSLClient client;
//HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

Adafruit_VS1053_FilePlayer musicPlayer =
  // create breakout-example object!
  Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);
// create shield-example object!
//Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);

void setup() {
  Serial.begin(9600);
  Serial.println("Adafruit VS1053 Simple Test");

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);  // print the network name (SSID);

    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  if (!musicPlayer.begin()) {  // initialise the music player
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1)
      ;
  }
  Serial.println(F("VS1053 found"));

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1)
      ;  // don't do anything more
  }


  // list files
  printDirectory(SD.open("/"), 0);

  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(20, 20);

  // Timer interrupts are not suggested, better to use DREQ interrupt!
  //musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int

  // If DREQ is on an interrupt pin (on uno, #2 or #3) we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

  //musicPlayer.startPlayingFile("/track002.mp3");
}

void loop() {

  if (state == 0) {
    getString = "";
    if (SD.exists("/report.wav")) { SD.remove("/report.wav"); };  // reset audio file
    Serial.println("connecting to server...");

    if (client.connectSSL("io.zongzechen.com", 443)) {
      client.println("GET /getAudio HTTP/1.1");
      client.println("Host: io.zongzechen.com");
      Serial.println("Host");
      client.println("Connection: close");
      client.println();
      Serial.println("Request sent");

      while (!client.available()) { delay(1); };  // wait for response;

      myFile = SD.open("/report.wav", FILE_WRITE);
      Serial.println("start writing file...");
      myFile.write("RIFF");  // write the wav header

      requestTime = millis();
      state++;  // move on to writing the file
    } else {
      Serial.println("connection failed");
      delay(200);  // try again later
    }

  }

  else if (state == 1) {
    while (client.available()) {  // if server returns content

      if (getString.indexOf("RIFF") == -1) {  // get rid of headers
        Serial.println("not found");
        char c = client.read();
        getString += c;
      } else {
        int length = client.available();
        byte buffer[length];
        client.readBytes(buffer, length);
        Serial.println(client.available());
        myFile.write(buffer, length);
      }
    }

    if (millis() - requestTime > (60 * 1000)) {
      Serial.println("Timer zero");
      myFile.close();  // close the file
      playStartTime = millis();
      state++;  // set timeOut of 1 minute
    }
  }

  else if (state == 2) {  // play the audio
    musicPlayer.startPlayingFile("/report.wav");
    Serial.println("now playing");

    state++;
  }

  else if (state == 3) {  // wait for next fetch
    Serial.println("waiting for next fetch...");
    delay(2 * 60 * 1000);
    state = 0;
  }
}


/// File listing helper
void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry = dir.openNextFile();
    if (!entry) {
      // no more files
      //Serial.println("**nomorefiles**");
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
