
#include "LoRaResponse.h"
#include "LoRaPacket.h"
#include <cstring> // Per memcpy

void printResponse(const LoRaResponsePacket &response)
{
    Serial.println("=== Risposta Inviata ===");
    Serial.printf("Tipo: 0x%02X\n", response.type);
    Serial.print("ID: ");
    for (int i = 0; i < 16; i++)
        Serial.printf("%02X", response.id[i]);                                         // Stampa ID byte per byte
    Serial.printf("\nLatitudine Parcheggio: %.7f\n", response.parking_latitude / 1e7); // Conversione Fixed-Point
    Serial.printf("Longitudine Parcheggio: %.7f\n", response.parking_longitude / 1e7);
    Serial.printf("Distanza: %u metri\n", response.distance);
    Serial.printf("Posti Disponibili: %u\n", response.available_slots);
    Serial.println("=========================");
}

// Funzione per preparare una risposta al sender
void generateResponse(
    const LoRaPacket &receivedPacket, // Pacchetto ricevuto dal sender
    float parkingLatitude,            // Latitudine del parcheggio
    float parkingLongitude,           // Longitudine del parcheggio
    uint16_t distance,                // Distanza in metri
    uint8_t availableSlots,           // Numero di posti disponibili
    LoRaResponsePacket &response      // Pacchetto di risposta da preparare
)
{
    response.type = 0x02;                                // Tipo messaggio: risposta
    memcpy(response.id, receivedPacket.id, 16);          // Copia l'ID dal pacchetto ricevuto
    response.parking_latitude = parkingLatitude * 1e7;   // Latitudine parcheggio (Fixed-Point)
    response.parking_longitude = parkingLongitude * 1e7; // Longitudine parcheggio (Fixed-Point)
    response.distance = distance;                        // Distanza in metri
    response.available_slots = availableSlots;           // Numero di posti disponibili
}