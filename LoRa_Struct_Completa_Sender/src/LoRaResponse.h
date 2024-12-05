// LoRaResponse.h

#ifndef LORA_RESPONSE_H
#define LORA_RESPONSE_H

#include <Arduino.h>

// Strutture dei pacchetti
struct LoRaResponsePacket
{                            // Pacchetto di risposta dal receiver al sender
  uint8_t type;              // Tipo di messaggio (ad esempio 0x02 per risposta)
  uint8_t id[16];            // ID univoco della richiesta
  int32_t parking_latitude;  // Latitudine del parcheggio in formato Fixed-Point
  int32_t parking_longitude; // Longitudine del parcheggio in formato Fixed-Point
  uint16_t distance;         // Distanza in metri
  uint8_t available_slots;   // Numero di posti disponibili
} __attribute__((packed));


void printResponse(const LoRaResponsePacket &response);

#endif // LORA_RESPONSE_H