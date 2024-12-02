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

// Inizializzazione display OLED e modulo LoRa
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, oled_scl, oled_sda, oled_rst);
SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);

// Struct compatta per il messaggio (2 byte)
struct Message {
  uint8_t type;  // Tipo di messaggio (1 byte)
  uint8_t id;    // ID del veicolo (1 byte)
} __attribute__((packed)); // Evita padding e garantisce dimensioni esatte


void updateDisplay(const char* type, uint8_t msgType, uint8_t msgID)
{
  u8g2.firstPage(); 
  do
  {
    u8g2.setFont(u8g2_font_6x10_tr); 
    u8g2.drawStr(0, 10, type);       // Mostra "Received" o messaggi di errore

    String line1 = "Type: 0x" + String(msgType, HEX); // Concatena "Type" e il valore esadecimale
    u8g2.drawStr(0, 20, line1.c_str());              // Mostra il tipo

    String line2 = "ID: " + String(msgID);           // Concatena "ID" e il valore numerico
    u8g2.drawStr(0, 30, line2.c_str());              // Mostra l'ID
  } while (u8g2.nextPage()); 
}

void setup()
{
  Serial.begin(9600); // Inizializza il monitor seriale
  u8g2.begin();       // Inizializza il display OLED

  // Configurazione del modulo LoRa
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE)
  {
    updateDisplay("LoRa Ready", 0, 0); // Mostra che il modulo è pronto
    Serial.println("[SX1262] Receiver initialized!");
  }
  else
  {
    updateDisplay("LoRa Error", 0, 0); // Segnala errore
    Serial.println("[SX1262] Initialization failed!");
    while (true); // Blocca l'esecuzione in caso di errore
  }

  // Impostazioni LoRa (devono essere identiche al sender)
  radio.setFrequency(868.0);       // Frequenza in MHz
  radio.setSpreadingFactor(10);    // Spreading Factor
  radio.setBandwidth(62.5);        // Larghezza di banda in kHz
  radio.setCodingRate(8);          // Coding Rate
}

void loop()
{
  // Buffer per il messaggio ricevuto
  Message msg;
  int state = radio.receive((uint8_t*)&msg, sizeof(msg)); // Riceve i dati nel buffer

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println("[SX1262] Message received!"); // Conferma ricezione

    // Decodifica e stampa i dati ricevuti
    Serial.print("Type: 0x");
    Serial.println(msg.type, HEX); // Mostra il tipo in esadecimale
    Serial.print("ID: ");
    Serial.println(msg.id); // Mostra l'ID numerico

    updateDisplay("Received", msg.type, msg.id); // Mostra sul display OLED
  }
  else if (state == RADIOLIB_ERR_RX_TIMEOUT)
  {
    Serial.println("[SX1262] Receive timeout!"); // Timeout ricezione
  }
  else if (state == RADIOLIB_ERR_CRC_MISMATCH)
  {
    Serial.println("[SX1262] CRC error!"); // Errore di integrità dei dati
  }
  else
  {
    Serial.print("[SX1262] Receive failed, code "); // Altri errori
    Serial.println(state);
  }

   radio.startReceive();
}

