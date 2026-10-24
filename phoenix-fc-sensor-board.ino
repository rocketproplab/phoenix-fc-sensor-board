// ─────────────────────────────────────────────────────────────────────────────
// Libraries
// ─────────────────────────────────────────────────────────────────────────────

// SPI Libs
#include <SPI.h>
// Ethernet Lib
#include "./src/w5500/w5500.h"
// Constants
#include "./src/constants.h"
// SD Card Lib
// #include <SD.h>
// File myFile;

// ─────────────────────────────────────────────────────────────────────────────
// Pin Definitions
// ─────────────────────────────────────────────────────────────────────────────

//Ethernet Chip Select
#define EthernetCS 10
#define SDCS 40

#include "src/pt/pt.h"
// ─────────────────────────────────────────────────────────────────────────────
// Ethernet (MAC‑RAW)
// ─────────────────────────────────────────────────────────────────────────────

uint8_t buffer[500];

// first byte 0x02 = locally‑administered, unicast
const uint8_t MAC_GROUND_STATION[6] = {0x02, 0x47, 0x53,
                                       0x00, 0x00, 0x01}; // GS:  ground station
const uint8_t MAC_SENSOR_GIGA[6] = {0x02, 0x53, 0x49,
                                    0x00, 0x00, 0x04}; // SI:  sensor interface

//
struct TelemetryFrame {
  // –––– Ethernet header (14 B) ––––
  uint8_t dstMac[6];
  uint8_t srcMac[6];
  uint16_t ethType; // always 0x8889

  // –––– Payload ––––
  uint8_t payload[200]; // 200 bytes usually enough.. optimize later
};
TelemetryFrame f;

// ─────────────────────────────────────────────────────────────────────────────
// Instance Declaration
// ─────────────────────────────────────────────────────────────────────────────
//Ethernet
Wiznet5500 w5500;
unsigned long lastSend = 0;

//Other Consts ***************************************************
float calibration_factor_star = -5600; //-7050 worked for my 440lb max scale setup
float calibration_factor_circle = -5900; //-7050 worked for my 440lb max scale setup
float calibration_factor_triangle = -6000; //-7050 worked for my 440lb max scale setup

// Ambient_Resolution ambientRes = RES_ZERO_POINT_0625;

void setup() {
  Serial.begin(115200);

  // Ethernet setup
  w5500.begin(MAC_SENSOR_GIGA);
  Serial.println("Ethernet Setup Complete");

  setupPTs();
  Serial.println("Done Initialized");
}

void sendSensorData() {
  // LIVE ETHERNET TRANSMISSION
  // Put all data into an output string to be sent over
  // String
  // dataOut=String(PT1)+","+String(PT2)+","+String(PT3)+","+String(PT4)+","+String(PT5);

  String dataOut;

  // construct PT data
  for (uint8_t ptID = 0; ptID < countPTs(); ++ptID) {
    double val = readPT(ptID);
    if (ptID > 0) dataOut += ",";
    dataOut += String(val);
  }

  Serial.println(dataOut);

  // Send data

  char *data = dataOut.c_str();
  int len = strlen(data);
  memset(&f, 0, sizeof(f)); // set to 0
  memcpy(f.dstMac, MAC_GROUND_STATION, 6);
  // ((byte *)&f)[0] = 0xFF;
  // ((byte *)&f)[1] = 0xFF;
  // ((byte *)&f)[2] = 0xFF;
  // ((byte *)&f)[3] = 0xFF;
  // ((byte *)&f)[4] = 0xFF;
  // ((byte *)&f)[5] = 0xFF;
  memcpy(f.srcMac, MAC_SENSOR_GIGA, 6);
  ((byte *)&f)[12] = 0x88;
  ((byte *)&f)[13] = 0x89;
  // Serial.print("Data len: ");
  // Serial.println(len);
  memcpy(f.payload, data, len);
  if (w5500.sendFrame((byte *)&f, sizeof(f)) <= 0) {
    Serial.println("Ethernet send Failed");
  }

  // myFile = SD.open("StaticFire.txt", FILE_WRITE);
  // if (myFile) {
  //   Serial.print("Writing to file...");
  //   myFile.println(dataOut);
  //   // close the file:
  //   myFile.close();
  //   Serial.println("done.");
  // } else {
  //   // if the file didn't open, print an error:
  //   Serial.println("error opening file");
  // }
}

void loop() {
  if (millis() - lastSend >= TELEMETRY_DELAY) {
    lastSend = millis();
    sendSensorData();
  }
}
