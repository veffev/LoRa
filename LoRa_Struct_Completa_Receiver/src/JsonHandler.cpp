#include "JsonHandler.h"
#include <FS.h>
#include <LittleFS.h>

// Carica il file JSON da LittleFS
String loadJsonFromLittleFS(const char *jsonFileName)
{
    File file = LittleFS.open(jsonFileName, "r");
    if (!file)
    {
        Serial.println("Errore nell'apertura del file JSON.");
        return "";
    }
    String json = file.readString();
    file.close();
    return json;
}

// Salva un JSON su LittleFS
void saveJsonToLittleFS(const char *jsonFileName, const String &json)
{
    File file = LittleFS.open(jsonFileName, "w");
    if (!file)
    {
        Serial.println("Errore nell'apertura del file per scrittura.");
        return;
    }
    file.print(json);
    file.close();
    Serial.println("JSON salvato correttamente.");
}

// Stampa il contenuto del file JSON
void printJsonFromLittleFS(const char *jsonFileName)
{
    String json = loadJsonFromLittleFS(jsonFileName);
    Serial.println("=== JSON Corrente ===");
    Serial.println(json);
    Serial.println("=====================");
}

// Elimina il file JSON
void deleteJsonFromLittleFS(const char *jsonFileName)
{
    if (LittleFS.exists(jsonFileName))
    {
        LittleFS.remove(jsonFileName);
        Serial.println("File JSON eliminato.");
    }
    else
    {
        Serial.println("File JSON non trovato.");
    }
}

// Crea un file JSON di default
void createDefaultJson(const char *jsonFileName)
{
    saveJsonToLittleFS(jsonFileName, R"json({
        "gateway": {
            "light_id": 1,
            "latitude": 37.7749,
            "longitude": 15.087,
            "parking_capacity": 3,
            "last_heartbeat": "2024-12-07T10:00:00Z"
        },
        "parking_spots": [
            {
                "parking_id": 1,
                "latitude": 37.7745,
                "longitude": 15.0875,
                "status": "available",
                "last_update": "2024-12-07T09:55:00Z"
            },
            {
                "parking_id": 2,
                "latitude": 37.7746,
                "longitude": 15.0876,
                "status": "occupied",
                "last_update": "2024-12-07T09:57:00Z"
            },
            {
                "parking_id": 3,
                "latitude": 37.7747,
                "longitude": 15.0877,
                "status": "available",
                "last_update": "2024-12-07T09:59:00Z"
            }
        ],
        "parking_assignments": []
    })json");
    Serial.println("File JSON di default creato.");
}