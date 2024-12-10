
// AckPacket.h
#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H

#include <Arduino.h>

// Dichiarazioni delle funzioni, accettando il nome del file come parametro
String loadJsonFromLittleFS(const char *jsonFileName);  // Carica il JSON persistente dal file
void saveJsonToLittleFS(const char *jsonFileName, const String &json);  // Salva un JSON nel file
void printJsonFromLittleFS(const char *jsonFileName);  // Stampa il contenuto del file JSON
void deleteJsonFromLittleFS(const char *jsonFileName); // Elimina il file JSON persistente
void createDefaultJson(const char *jsonFileName);      // Crea un file JSON di default

#endif // JSON_HANDLER_H