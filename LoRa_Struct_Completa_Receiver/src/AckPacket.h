
// AckPacket.h
#ifndef ACK_PACKET_H
#define ACK_PACKET_H

#include <Arduino.h>

struct AckPacket
{                   // Pacchetto ACK inviato dal receiver al sender
    uint8_t id[16]; // ID univoco della richiesta
} __attribute__((packed));

void printAck(const AckPacket &ack);

#endif // ACK_PACKET_H