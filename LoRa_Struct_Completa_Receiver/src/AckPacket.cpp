#include "AckPacket.h"

void printAck(const AckPacket &ack)
{
    Serial.println("=== ACK Inviato ===");
    Serial.print("ID: ");
    for (int i = 0; i < 16; i++)
        Serial.printf("%02X", ack.id[i]); // Stampa ID byte per byte
    Serial.println();
    Serial.println("=====================");
}