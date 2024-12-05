#include "LoRaResponse.h"

void printResponse(const LoRaResponsePacket &response)
{
  Serial.println("[SX1262] Risposta ricevuta!");
  Serial.println("=== Risposta ===");
  Serial.printf("Parcheggio: %.7f, %.7f\n", response.parking_latitude / 1e7, response.parking_longitude / 1e7);
  Serial.printf("Distanza: %d metri\n", response.distance);
  Serial.printf("Posti disponibili: %d\n", response.available_slots);
}