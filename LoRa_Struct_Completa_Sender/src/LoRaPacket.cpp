#include "LoRaPacket.h"
#include <MD5Builder.h>

// Funzione per generare un ID temporaneo utilizzando MD5
void generateTemporaryID(const char *plate, uint32_t timestamp, uint8_t *outputID)
{
  String input = String(plate) + String(timestamp); // Combina la targa del veicolo con il timestamp
  MD5Builder md5;
  md5.begin();            // Inizia il calcolo MD5
  md5.add(input);         // Aggiunge la stringa combinata
  md5.calculate();        // Calcola l'hash MD5
  md5.getBytes(outputID); // Salva il risultato nell'output
}

// Funzione per convertire una coordinata GPS da float a Fixed-Point
int32_t convertToFixedPoint(float coordinate)
{
  return (int32_t)(coordinate * 1e7); // Moltiplica per 10^7 per aumentare la precisione
}

//Genere messaggio di richiesta
void generateRequest(const char *vehiclePlate, float latitude, float longitude, uint8_t requestStatus, LoRaPacket &packet)
{
  // Generazione di un ID temporaneo basato sulla targa e sul timestamp
  uint32_t timestamp = millis(); // Timestamp corrente
  generateTemporaryID(vehiclePlate, timestamp, packet.id);

  // Conversione dei dati e popolamento del pacchetto
  packet.type = 0x01;                                // Tipo di pacchetto (richiesta)
  packet.latitude = convertToFixedPoint(latitude);   // Latitudine in formato Fixed-Point
  packet.longitude = convertToFixedPoint(longitude); // Longitudine in formato Fixed-Point
  packet.request_status = requestStatus;             // Stato della richiesta
  packet.timestamp = timestamp;                      // Timestamp corrente
}

void printPacket(const LoRaPacket &packet)
{
  // Stampa dettagli del pacchetto inviato
  Serial.println("=== Pacchetto Inviato ===");
  Serial.printf("Tipo: 0x%02X\n", packet.type);
  Serial.print("ID: ");
  for (int i = 0; i < 16; i++)
    Serial.printf("%02X", packet.id[i]);
  Serial.printf("\nLatitudine: %.7f\n", packet.latitude / 1e7);
  Serial.printf("Longitudine: %.7f\n", packet.longitude / 1e7);
  Serial.printf("Stato Richiesta: 0x%02X\n", packet.request_status);
  Serial.printf("Timestamp: %lu\n", packet.timestamp);
  Serial.println("=========================");
}