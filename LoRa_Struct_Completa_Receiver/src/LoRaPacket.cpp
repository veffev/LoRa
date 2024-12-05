#include "LoRaPacket.h"

void printReceivedPacket(const LoRaPacket& packet) {
  Serial.println("=== Pacchetto Ricevuto ===");
  Serial.printf("Tipo: 0x%02X\n", packet.type);
  Serial.print("ID: ");
  for (int i = 0; i < 16; i++) Serial.printf("%02X", packet.id[i]); // Stampa ID byte per byte
  Serial.printf("\nLatitudine: %.7f\n", packet.latitude / 1e7); // Conversione Fixed-Point in float
  Serial.printf("Longitudine: %.7f\n", packet.longitude / 1e7);
  Serial.printf("Stato Richiesta: 0x%02X\n", packet.request_status);
  Serial.printf("Timestamp: %lu\n", packet.timestamp);
  Serial.println("=========================");
}
