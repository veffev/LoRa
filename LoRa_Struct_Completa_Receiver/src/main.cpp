#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <RadioLib.h>
#include "LoRaPacket.h"
#include "LoRaResponse.h"
#include "AckPacket.h"
#include <cstring> // Per memcpy

// Pin OLED e LoRa
#define oled_scl 18
#define oled_sda 17
#define oled_rst 21
#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13

// Inizializzazione del display OLED e del modulo LoRa
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, oled_scl, oled_sda, oled_rst);
SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);

// Strutture dati per il protocollo di comunicazione

// Funzione per assicurarsi che il modulo LoRa sia in modalità ricezione
void ensureReceiveMode();

void setup()
{
  Serial.begin(115200); // Inizializza il monitor seriale per il debug
  u8g2.begin();         // Inizializza il display OLED

  // Configura il modulo LoRa
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE)
  {                                                     // Controlla se la configurazione è riuscita
    Serial.println("[SX1262] Receiver inizializzato!"); // Successo
  }
  else
  {
    Serial.printf("[SX1262] Errore di inizializzazione: %d\n", state);
    while (true)
      ; // Si blocca in caso di errore
  }

  // Configura i parametri di comunicazione LoRa
  radio.setFrequency(868.0);    // Frequenza in MHz
  radio.setSpreadingFactor(10); // Spreading Factor
  radio.setBandwidth(62.5);     // Banda
  radio.setCodingRate(8);       // Coding Rate
  ensureReceiveMode();          // Avvia la modalità ricezione
}

void loop()
{
  LoRaPacket packet;

  // Riceve un pacchetto dal sender
  int state = radio.receive((uint8_t *)&packet, sizeof(packet));
  if (state == RADIOLIB_ERR_NONE)
  { // Verifica che il pacchetto sia stato ricevuto correttamente
    if (packet.type == 0x01)
    { // Controlla il tipo di messaggio
      Serial.println("[SX1262] Richiesta ricevuta!");
      printReceivedPacket(packet); // Stampa dettagli del pacchetto ricevuto

      // Prepara e invia un ACK al sender
      AckPacket ack;
      memcpy(ack.id, packet.id, 16);                        // Copia l'ID dal pacchetto ricevuto
      state = radio.transmit((uint8_t *)&ack, sizeof(ack)); // Invia l'ACK
      if (state == RADIOLIB_ERR_NONE)
      { // Verifica che l'invio sia riuscito
        Serial.println("[SX1262] ACK inviato!");
        printAck(ack); // Stampa dettagli dell'ACK inviato
      }
      ensureReceiveMode(); // Torna in modalità ricezione

      // Prepara la risposta al sender
      LoRaResponsePacket response;

      // Parametri della risposta FAKE
      float parkingLatitude = 37.507;  // Latitudine parcheggio
      float parkingLongitude = 15.092; // Longitudine parcheggio
      uint16_t distance = 50;          // Distanza in metri
      uint8_t availableSlots = 10;     // Numero di posti disponibili

      // Preparazione della risposta
      generateResponse(packet, parkingLatitude, parkingLongitude, distance, availableSlots, response);

      delay(5000); // Simula il tempo di elaborazione

      // Invia la risposta al sender
      state = radio.transmit((uint8_t *)&response, sizeof(response)); // Invio della risposta
      if (state == RADIOLIB_ERR_NONE)
      { // Verifica che l'invio sia riuscito
        Serial.println("[SX1262] Risposta inviata!");
        printResponse(response); // Stampa dettagli della risposta inviata
      }
      ensureReceiveMode(); // Torna in modalità ricezione
    }
  }
}

void ensureReceiveMode()
{
  int state = radio.startReceive(); // Avvia la modalità ricezione
  if (state != RADIOLIB_ERR_NONE)
  { // Verifica eventuali errori
    Serial.printf("[SX1262] Errore rientro in ricezione: %d\n", state);
  }
  else
  {
    Serial.println("[SX1262] Receiver in modalità ascolto."); // Ricezione avviata correttamente
  }
}