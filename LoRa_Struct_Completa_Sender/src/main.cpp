#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <RadioLib.h>
#include <MD5Builder.h>
#include "LoRaPacket.h"
#include "LoRaResponse.h"

// Pin OLED e LoRa
#define oled_scl 18
#define oled_sda 17
#define oled_rst 21
#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13

int count = 0;

// Inizializzazione del display OLED e del modulo LoRa
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, oled_scl, oled_sda, oled_rst);
SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);

struct AckPacket
{                 // Pacchetto ACK inviato dal receiver al sender
  uint8_t id[16]; // ID univoco della richiesta
} __attribute__((packed));

// Funzione per attendere una risposta dal receiver entro un timeout,
// e mi permettte di verificare che il pacchetto ricevuto sia il dato atteso
bool waitForResponse(uint32_t timeout, LoRaResponsePacket &response, uint8_t *expectedID);

void setup()
{
  Serial.begin(115200); // Inizializza la comunicazione seriale per il debug
  u8g2.begin();         // Inizializza il display OLED

  // Configura il modulo LoRa
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println("[SX1262] Sender inizializzato!"); // Messaggio di successo
  }
  else
  {
    Serial.printf("[SX1262] Errore di inizializzazione: %d\n", state);
    while (true)
      ; // Si blocca in caso di errore
  }

  // Imposta i parametri di comunicazione LoRa
  radio.setFrequency(868.0);    // Frequenza in MHz
  radio.setSpreadingFactor(10); // Spreading Factor
  radio.setBandwidth(62.5);     // Banda
  radio.setCodingRate(8);       // Coding Rate
}

void loop()
{

  // Creazione del pacchetto di richiesta
  LoRaPacket packet;
  packet.type = 0x01; // Tipo di messaggio: richiesta di parcheggio

  // Fake Data
  const char *vehiclePlate = "ABC123"; // Targa del veicolo
  float latitude = 37.502;             // Latitudine simulata
  float longitude = 15.087;            // Longitudine simulata
  uint8_t requestStatus = 0x00;        // Stato richiesta: attiva

  // Generazione del pacchetto di richiesta
  generateRequest(vehiclePlate, latitude, longitude, requestStatus, packet);

  // stampo pacchetto di richiesta
  printPacket(packet);

  // Invio del pacchetto al receiver
  int state = radio.transmit((uint8_t *)&packet, sizeof(packet));
  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println("[SX1262] Richiesta inviata con successo!");

    // Ora il transmitter deve tornare in ricezione
    state = radio.startReceive();
    if (state != RADIOLIB_ERR_NONE)
    {
      Serial.printf("[SX1262] Errore rientro in ricezione: %d\n", state);
    }

    // Attende l'ACK dal receiver
    AckPacket ack;
    unsigned long startTime = millis();
    bool ackReceived = false;

    while (millis() - startTime < 5000)
    { // Timeout di 5 secondi per l'ACK
      state = radio.receive((uint8_t *)&ack, sizeof(ack));
      if (state == RADIOLIB_ERR_NONE && memcmp(ack.id, packet.id, 16) == 0)
      {
        // Verifica che l'ID nell'ACK corrisponda a quello inviato
        Serial.println("[SX1262] ACK ricevuto!");
        ackReceived = true;
        break;
      }
    }

    if (ackReceived)
    {
      // Attesa della risposta dal receiver
      LoRaResponsePacket response;
      if (waitForResponse(50000, response, packet.id))
      { // Timeout di 50 secondi
        printResponse(response);
      }
      else
      {
        Serial.println("[SX1262] Risposta non ricevuta entro il timeout!");
      }
    }
    else
    {
      Serial.println("[SX1262] ACK non ricevuto entro il timeout!");
    }
  }
  else
  {
    // Stampa eventuali errori durante l'invio
    Serial.printf("[SX1262] Errore durante l'invio: %d\n", state);
  }

  delay(5000); // Attesa prima di inviare una nuova richiesta
}

// Funzione per attendere una risposta dal receiver entro un timeout, e mi permettte di verificare che il pacchetto ricevuto sia il dato atteso
bool waitForResponse(uint32_t timeout, LoRaResponsePacket &response, uint8_t *expectedID)
{
  unsigned long startTime = millis(); // Tempo iniziale
  while (millis() - startTime < timeout)
  {                                                                    // Continua a ricevere fino al timeout
    int state = radio.receive((uint8_t *)&response, sizeof(response)); // Riceve il pacchetto
    if (state == RADIOLIB_ERR_NONE)
    { // Controlla se la ricezione è avvenuta con successo
      if (response.type == 0x02 && memcmp(response.id, expectedID, 16) == 0)
      {
        // Verifica che il tipo sia corretto e che l'ID corrisponda
        return true;
      }
      // se ho altri tipi di codice che il receiver potrebbe mandare allora posso effettuare i controlli con else if,
      // ad esemppio codice parcheggio non disponibile
    }
    else if (state != RADIOLIB_ERR_RX_TIMEOUT)
    { // Stampa errori diversi dal timeout
      Serial.printf("[SX1262] Errore durante la ricezione: %d\n", state);
      break;
    }
  }
  return false; // Ritorna false se il timeout è stato superato o c'è stato un errore
}
