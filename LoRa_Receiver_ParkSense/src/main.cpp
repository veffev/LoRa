#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <RadioLib.h>

// Pin OLED e LoRa
#define oled_scl 18
#define oled_sda 17
#define oled_rst 21
#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13

U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, oled_scl, oled_sda, oled_rst);
SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);

void updateReceiverDisplay(String type, String vehicleID, String data, String timestamp) {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 10, "Received");
    String line1 = "Type: " + type;
    u8g2.drawStr(0, 20, line1.c_str());
    String line2 = "Vehicle: " + vehicleID;
    u8g2.drawStr(0, 30, line2.c_str());
    String line3 = "Data: " + data;
    u8g2.drawStr(0, 40, line3.c_str());
    String line4 = "Timestamp: ";
    u8g2.drawStr(0, 50, line4.c_str());
    String line5 = timestamp;
    u8g2.drawStr(0, 60, line5.c_str());
  } while (u8g2.nextPage());
}

void parseMessage(String message) {
  int separatorIndex;
  String messageType, vehicleID, data, timestamp;

  separatorIndex = message.indexOf('|');
  messageType = message.substring(0, separatorIndex);
  message = message.substring(separatorIndex + 1);

  separatorIndex = message.indexOf('|');
  vehicleID = message.substring(0, separatorIndex);
  message = message.substring(separatorIndex + 1);

  separatorIndex = message.indexOf('|');
  data = message.substring(0, separatorIndex);
  timestamp = message.substring(separatorIndex + 1);

  // Log dei dettagli del messaggio
  Serial.println("Message Type: " + messageType);
  Serial.println("Vehicle ID: " + vehicleID);
  Serial.println("Data: " + data);
  Serial.println("Timestamp: " + timestamp);

  // Aggiorna il display OLED
  updateReceiverDisplay(messageType, vehicleID, data, timestamp);
}

void setup() {
  Serial.begin(9600);
  u8g2.begin();

  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    updateReceiverDisplay("LoRa Ready", "Receiver", "Initialized", "");
  } else {
    updateReceiverDisplay("LoRa Error", "Check", "Connections", "");
    while (true);
  }

  radio.setFrequency(868.0);
  radio.setSpreadingFactor(10);
  radio.setBandwidth(62.5);
  radio.setCodingRate(8);
  radio.setOutputPower(20);
}

void loop() {
  Serial.println("[SX1262] Waiting for message...");

  uint8_t buffer[64] = {0};
  int state = radio.receive(buffer, sizeof(buffer));

  if (state == RADIOLIB_ERR_NONE) {
    String message = String((char*)buffer);
    Serial.println("[SX1262] Message received successfully!");
    Serial.println("Content: " + message);
    Serial.print("RSSI: ");
    Serial.print(radio.getRSSI());
    Serial.println(" dBm");
    Serial.print("SNR: ");
    Serial.print(radio.getSNR());
    Serial.println(" dB");
    parseMessage(message);
  } else {
    Serial.println("[SX1262] Timeout or error while receiving message.");
  }

  // Rimane in ascolto senza delay
  radio.startReceive();
}