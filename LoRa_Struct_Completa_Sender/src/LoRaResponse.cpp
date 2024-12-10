#include "LoRaResponse.h"

void printResponse(const LoRaResponsePacket &response)
{
    Serial.println("=== Risposta ===");
    Serial.printf("Tipo: 0x%02X\n", response.type); // Stampa il tipo di messaggio
    Serial.print("ID: ");
    for (int i = 0; i < 16; i++)
    {
        Serial.printf("%02X", response.id[i]); // Stampa l'ID byte per byte
    }
    Serial.println();
    Serial.printf("Parcheggio: %.7f, %.7f\n", 
                  response.parking_latitude / 1e7, 
                  response.parking_longitude / 1e7); // Stampa latitudine e longitudine
    Serial.printf("Distanza: %u metri\n", response.distance); // Stampa la distanza
    Serial.printf("Stato Richiesta: 0x%02X\n", response.request_status); // Stampa lo stato della richiesta
    Serial.println("=========================");
}