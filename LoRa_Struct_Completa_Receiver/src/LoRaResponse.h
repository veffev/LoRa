#ifndef LORA_RESPONSE_H
#define LORA_RESPONSE_H

#include <Arduino.h>
#include "LoRaPacket.h"

// Strutture dei pacchetti
struct LoRaResponsePacket
{                              // Pacchetto di risposta dal receiver al sender
    uint8_t type;              // Tipo di messaggio (ad esempio 0x02 per risposta)
    uint8_t id[16];            // ID univoco della richiesta
    int32_t parking_latitude;  // Latitudine del parcheggio in formato Fixed-Point
    int32_t parking_longitude; // Longitudine del parcheggio in formato Fixed-Point
    uint16_t distance;         // Distanza in metri
    uint8_t request_status;    // Stato della richiesta (0x00: nessun parcheggio, 0x01: parcheggio trovato, ecc.)
} __attribute__((packed));

void printResponse(const LoRaResponsePacket &response);

void generateResponse(
  const LoRaPacket& receivedPacket, // Pacchetto ricevuto dal sender
  float parkingLatitude,            // Latitudine del parcheggio
  float parkingLongitude,           // Longitudine del parcheggio
  uint16_t distance,                // Distanza in metri
  uint8_t requestStatus,            // Stato della richiesta
  LoRaResponsePacket& response      // Pacchetto di risposta da preparare
);

#endif // LORA_RESPONSE_H