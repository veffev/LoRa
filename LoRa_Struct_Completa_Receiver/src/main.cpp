#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <RadioLib.h>
#include "LoRaPacket.h"
#include "LoRaResponse.h"
#include "AckPacket.h"
#include <cstring> // Per memcpy
#include <FS.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <cmath>
#include <float.h>

#include <JsonHandler.h>

// Costanti per il calcolo delle distanze geografiche
#define EARTH_RADIUS 6371.0 // Raggio della Terra in km

// Pin utilizzati per OLED e LoRa
#define oled_scl 18
#define oled_sda 17
#define oled_rst 21
#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13

// Inizializzazione dei moduli OLED e LoRa
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, oled_scl, oled_sda, oled_rst);
SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);

// Nome del file JSON persistente
const char *jsonFileName = "/config.json";

// Dichiarazione delle funzioni
void ensureReceiveMode();                                                 // Assicura che il modulo LoRa sia in modalità ricezione
double haversineDistance(float lat1, float lon1, float lat2, float lon2); // Calcola la distanza tra due coordinate geografiche

void setup()
{
  // Inizializza la comunicazione seriale per il debug
  Serial.begin(115200);

  // Inizializza il display OLED
  u8g2.begin();

  // Inizializza LittleFS per la gestione dei file
  if (!LittleFS.begin(true))
  {
    Serial.println("Errore nell'inizializzazione di LittleFS.");
    while (true)
      ;
  }

  // Elimina il file JSON (opzionale, utile per ripristinare lo stato iniziale)
  deleteJsonFromLittleFS(jsonFileName);

  // Se il file JSON non esiste, crea il file di default
  if (!LittleFS.exists(jsonFileName))
  {
    Serial.println("File JSON non trovato. Creazione del file di default...");
    createDefaultJson(jsonFileName);
  }

  // Configura il modulo LoRa
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println("[SX1262] Receiver inizializzato!");
  }
  else
  {
    Serial.printf("[SX1262] Errore di inizializzazione: %d\n", state);
    while (true)
      ;
  }

  // Configura i parametri del modulo LoRa
  radio.setFrequency(868.0);    // Frequenza di lavoro (in MHz)
  radio.setSpreadingFactor(10); // Spreading Factor
  radio.setBandwidth(62.5);     // Banda (in kHz)
  radio.setCodingRate(8);       // Coding Rate

  // Imposta il modulo LoRa in modalità ricezione
  ensureReceiveMode();
}

void loop()
{
  LoRaPacket packet;

  // Prova a ricevere un pacchetto
  int state = radio.receive((uint8_t *)&packet, sizeof(packet));
  if (state == RADIOLIB_ERR_NONE)
  {
    // Controlla il tipo di pacchetto ricevuto
    if (packet.type == 0x01) // Tipo 0x01: Richiesta di parcheggio
    {
      Serial.println("[SX1262] Richiesta ricevuta!");
      printReceivedPacket(packet);

      // Prepara e invia un ACK al mittente
      AckPacket ack;
      memcpy(ack.id, packet.id, 16);
      state = radio.transmit((uint8_t *)&ack, sizeof(ack));
      if (state == RADIOLIB_ERR_NONE)
      {
        Serial.println("[SX1262] ACK inviato!");
        printAck(ack);
      }
      else
      {
        Serial.printf("[SX1262] Errore durante l'invio dell'ACK: %d\n", state);
      }

      // Ritorna in modalità ricezione
      ensureReceiveMode();

      // Carica il JSON persistente dal file
      String json = loadJsonFromLittleFS(jsonFileName);
      if (json.isEmpty())
      {
        Serial.println("Errore: JSON vuoto o non trovato.");
        ensureReceiveMode();
        return;
      }

      // Parsing del JSON
      DynamicJsonDocument doc(4096);
      DeserializationError error = deserializeJson(doc, json);
      if (error)
      {
        Serial.println("Errore nel parsing del JSON.");
        ensureReceiveMode();
        return;
      }

      // Recupera gli array di parcheggi e assegnazioni
      JsonArray parkingSpots = doc["parking_spots"];
      JsonArray assignments = doc["parking_assignments"];
      if (parkingSpots.isNull())
      {
        Serial.println("Errore: 'parking_spots' non trovato nel JSON.");
        ensureReceiveMode();
        return;
      }

      // Cerca il parcheggio disponibile più vicino
      JsonObject closestSpot;
      float minDistance = FLT_MAX;

      for (JsonObject spot : parkingSpots)
      {
        bool isAssigned = false;

        // Controlla se il parcheggio è già assegnato
        if (!assignments.isNull())
        {
          for (JsonObject assignment : assignments)
          {
            if (assignment["parking_spot"] == spot["parking_id"])
            {
              isAssigned = true;
              break;
            }
          }
        }

        // Considera solo parcheggi disponibili e non assegnati
        if (strcmp(spot["status"], "available") == 0 && !isAssigned)
        {
          float distance = haversineDistance(packet.latitude, packet.longitude,
                                             spot["latitude"], spot["longitude"]);
          if (distance < minDistance)
          {
            minDistance = distance;
            closestSpot = spot;
          }
        }
      }

      if (!closestSpot.isNull())
      {
        // Prepara i dati per la risposta
        float parkingLatitude = closestSpot["latitude"];
        float parkingLongitude = closestSpot["longitude"];
        uint16_t distance = static_cast<uint16_t>(minDistance);

        LoRaResponsePacket response;
        generateResponse(packet, parkingLatitude, parkingLongitude, distance, 0x01, response);

        // Aggiunge una nuova assegnazione
        JsonObject newAssignment = assignments.createNestedObject();
        newAssignment["vehicle_id"] = String(packet.id, HEX);
        newAssignment["parking_spot"] = closestSpot["parking_id"];
        newAssignment["assigned_at"] = "2024-12-07T12:00:00Z"; // Timestamp esempio
        newAssignment["status"] = "assigned";

        // Salva il JSON aggiornato
        String updatedJson;
        serializeJson(doc, updatedJson);
        saveJsonToLittleFS(jsonFileName, updatedJson);

        // Stampa il JSON aggiornato
        printJsonFromLittleFS(jsonFileName);

        // Invia la risposta
        state = radio.transmit((uint8_t *)&response, sizeof(response));
        if (state == RADIOLIB_ERR_NONE)
        {
          Serial.println("[SX1262] Risposta inviata!");
          printResponse(response);
        }
        else
        {
          Serial.printf("[SX1262] Errore durante l'invio della risposta: %d\n", state);
        }
      }
      else
      {
        // Nessun parcheggio disponibile
        Serial.println("Nessun parcheggio disponibile trovato.");
        LoRaResponsePacket response;
        generateResponse(packet, 0.0, 0.0, 0, 0x00, response);

        state = radio.transmit((uint8_t *)&response, sizeof(response));
        if (state == RADIOLIB_ERR_NONE)
        {
          Serial.println("[SX1262] Risposta di 'nessun parcheggio disponibile' inviata!");
          printResponse(response);
        }
        else
        {
          Serial.printf("[SX1262] Errore durante l'invio della risposta: %d\n", state);
        }
      }
    }
  }
  else if (state != RADIOLIB_ERR_RX_TIMEOUT)
  {
    Serial.printf("[SX1262] Errore durante la ricezione: %d\n", state);
  }

  // Torna in modalità ricezione
  ensureReceiveMode();
}

// Assicura che il modulo LoRa sia in modalità ricezione
void ensureReceiveMode()
{
  int state = radio.startReceive();
  if (state != RADIOLIB_ERR_NONE)
  {
    Serial.printf("[SX1262] Errore rientro in ricezione: %d\n", state);
  }
  else
  {
    Serial.println("[SX1262] Receiver in modalità ascolto.");
  }
}

// Calcola la distanza tra due coordinate geografiche (formula Haversine)
double haversineDistance(float lat1, float lon1, float lat2, float lon2)
{
  double dLat = radians(lat2 - lat1);
  double dLon = radians(lon2 - lon1);
  double a = sin(dLat / 2) * sin(dLat / 2) +
             cos(radians(lat1)) * cos(radians(lat2)) *
                 sin(dLon / 2) * sin(dLon / 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  return EARTH_RADIUS * c * 1000; // Distanza in metri
}
