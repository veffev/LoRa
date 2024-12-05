
// LoRaPacket.h
#ifndef LORA_PACKET_H
#define LORA_PACKET_H

#include <Arduino.h>

// Strutture dei pacchetti
struct LoRaPacket { // Pacchetto inviato dal sender al receiver
  uint8_t type;                // Tipo di messaggio (ad esempio 0x01 per richiesta)
  uint8_t id[16];              // ID univoco del pacchetto
  int32_t latitude;            // Latitudine in formato Fixed-Point
  int32_t longitude;           // Longitudine in formato Fixed-Point
  uint8_t request_status;      // Stato della richiesta (es. 0x01 per attiva)
  uint32_t timestamp;          // Timestamp della richiesta
} __attribute__((packed));


void printReceivedPacket(const LoRaPacket& packet);

#endif // LORA_PACKET_H