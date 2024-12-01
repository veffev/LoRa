#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <RadioLib.h>

// Definizione codici messaggio
#define MSG_TYPE_REQUEST "0x01"
#define MSG_TYPE_ASSIGN "0x02"
#define MSG_TYPE_OCCUPIED "0x03"
#define MSG_TYPE_RELEASED "0x04"

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

unsigned int messageCount = 0;

void updateDisplay(String type, String message, int msgNum)
{
  u8g2.firstPage();
  do
  {
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(0, 10, type.c_str());
    String line1 = "Type: " + message;
    u8g2.drawStr(0, 20, line1.c_str());
    String line2 = "Msg #: " + String(msgNum);
    u8g2.drawStr(0, 30, line2.c_str());
  } while (u8g2.nextPage());
}

String createMessage(String messageType, String vehicleID, String data)
{
  String timestamp = String(millis());
  return messageType + "|" + vehicleID + "|" + data + "|" + timestamp;
}

void setup()
{
  Serial.begin(9600);
  u8g2.begin();

  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE)
  {
    updateDisplay("LoRa Ready", "Sender Initialized", messageCount);
  }
  else
  {
    updateDisplay("LoRa Error", "Check Connections", messageCount);
    while (true)
      ; // Blocca l'esecuzione in caso di errore
  }

  radio.setFrequency(868.0);
  radio.setSpreadingFactor(10);
  radio.setBandwidth(62.5);
  radio.setCodingRate(8);
  radio.setOutputPower(20);

  // Ritardo iniziale per sincronizzare il receiver
  delay(5000);
}

void loop()
{
  String messageType;
  String vehicleID = "V123";
  String data = "37.7749|-122.4194";

  // Alterna i tipi di messaggi
  switch (messageCount % 4)
  {
  case 0:
    messageType = MSG_TYPE_REQUEST;
    break;
  case 1:
    messageType = MSG_TYPE_ASSIGN;
    break;
  case 2:
    messageType = MSG_TYPE_OCCUPIED;
    break;
  case 3:
    messageType = MSG_TYPE_RELEASED;
    break;
  }

  String message = createMessage(messageType, vehicleID, data);

  Serial.print("[SX1262] Sending message: ");
  Serial.println(message);

  int state = radio.transmit(message);

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println("[SX1262] Message sent successfully!");
    updateDisplay("Sent", messageType, messageCount);
  }
  else
  {
    Serial.println("[SX1262] Failed to send message!");
    updateDisplay("Send Error", "Check Connections", messageCount);
  }

  messageCount++;
  delay(5000);
}