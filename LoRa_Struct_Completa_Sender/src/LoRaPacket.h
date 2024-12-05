// LoRaPacket.h

#ifndef LORA_PACKET_H
#define LORA_PACKET_H

#include <Arduino.h>

// Strutture dei pacchetti
struct LoRaPacket
{                         // Pacchetto inviato dal sender al receiver
  uint8_t type;           // Tipo di messaggio (ad esempio 0x01 per richiesta)
  uint8_t id[16];         // ID univoco generato per il pacchetto
  int32_t latitude;       // Latitudine in formato Fixed-Point
  int32_t longitude;      // Longitudine in formato Fixed-Point
  uint8_t request_status; // Stato della richiesta (ad esempio 0x01 per attiva)
  uint32_t timestamp;     // Timestamp della richiesta
} __attribute__((packed));

void generateTemporaryID(const char* plate, uint32_t timestamp, uint8_t* outputID);
int32_t convertToFixedPoint(float coordinate);
void generateRequest(const char* vehiclePlate, float latitude, float longitude, uint8_t requestStatus, LoRaPacket& packet);

void printPacket(const LoRaPacket &packet);

#endif // LORA_PACKET_H