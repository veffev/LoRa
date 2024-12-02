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

unsigned int messageCount = 0; // Contatore messaggi inviati

// Struct compatta per il messaggio (2 byte)
struct Message {
  uint8_t type;  // Tipo di messaggio (1 byte)
  uint8_t id;    // ID del veicolo (1 byte)
} __attribute__((packed)); // Evita padding e garantisce dimensioni esatte


void updateDisplay(const char* type, uint8_t id)
{
  u8g2.firstPage(); // Inizia la scrittura sul display
  do
  {
    u8g2.setFont(u8g2_font_6x10_tr); // Seleziona il font
    u8g2.drawStr(0, 10, type);       // Mostra il tipo di messaggio

    String line1 = "ID: " + String(id); // Concatena "ID" e il valore numerico
    u8g2.drawStr(0, 20, line1.c_str()); // Mostra l'ID
  } while (u8g2.nextPage()); // Completa la scrittura
}

void setup()
{
  Serial.begin(9600); // Inizializza il monitor seriale
  u8g2.begin();       // Inizializza il display OLED

  // Configurazione del modulo LoRa
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE)
  {
    updateDisplay("LoRa Ready", 0); // Mostra che il modulo è pronto
    Serial.println("[SX1262] Sender initialized!");
  }
  else
  {
    updateDisplay("LoRa Error", 0); // Segnala errore
    Serial.println("[SX1262] Initialization failed!");
    while (true); // Blocca l'esecuzione in caso di errore
  }

  // Impostazioni LoRa (devono essere identiche sul receiver)
  radio.setFrequency(868.0);       // Frequenza in MHz
  radio.setSpreadingFactor(10);    // Spreading Factor
  radio.setBandwidth(62.5);        // Larghezza di banda in kHz
  radio.setCodingRate(8);          // Coding Rate
  radio.setOutputPower(20);        // Potenza di trasmissione in dBm

  delay(5000); // Ritardo iniziale per stabilire connessioni
}

void loop()
{
  // Crea un messaggio 
  Message msg;

  // Imposta l'ID del veicolo (un valore numerico)
  msg.id = messageCount % 256; // Ciclo per ID da 0 a 255

  // Alterna il tipo di messaggio
  switch (messageCount % 4)
  {
  case 0:
    msg.type = 0x01; // Richiesta (MSG_TYPE_REQUEST)
    break;
  case 1:
    msg.type = 0x02; // Assegnazione (MSG_TYPE_ASSIGN)
    break;
  case 2:
    msg.type = 0x03; // Occupazione (MSG_TYPE_OCCUPIED)
    break;
  case 3:
    msg.type = 0x04; // Rilascio (MSG_TYPE_RELEASED)
    break;
  }

  // Mostra i dati nel monitor seriale
  Serial.print("[SX1262] Sending message: Type=0x");
  Serial.print(msg.type, HEX);
  Serial.print(", ID=");
  Serial.println(msg.id);

  // Trasmette il messaggio come buffer binario
  int state = radio.transmit((uint8_t*)&msg, sizeof(msg));

  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println("[SX1262] Message sent successfully!"); // Conferma invio
    updateDisplay("Sent", msg.id); // Mostra sul display
  }
  else
  {
    Serial.println("[SX1262] Failed to send message!"); // Segnala errore
    updateDisplay("Send Error", 0);
  }

  messageCount++; // Incrementa il contatore dei messaggi
  delay(5000); // Ritardo tra i messaggi
}