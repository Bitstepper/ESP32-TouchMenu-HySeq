// radar_handler.cpp
#include "radar_handler.h"
#include <Wire.h>
#include "SparkFun_Qwiic_XM125_Arduino_Library.h"
#include <SimpleKalmanFilter.h>

// === COSTANTI LOCALI ===
static const uint8_t RADAR_I2C_ADDRESS = 0x52;
static const uint8_t DEFAULT_MAX_PROFILE = 2;
static const float DEFAULT_SMOOTHING = 0.2f;
static const uint32_t DEFAULT_START_MM = 250;
static const uint32_t DEFAULT_END_MM = 1500;

// === OGGETTI GLOBALI ===
static SparkFunXM125DistanceV1 radarSensor;
static SimpleKalmanFilter kalmanFilter(20, 10, 0.1);

// === VARIABILI DI STATO ===
static bool radarReady = false;
static float rawDistance = 0;
static float kalmanFiltered = 0;
static float filteredDistance = 0;
static bool firstReading = true;

// Configurazione
static uint32_t rangeStart = DEFAULT_START_MM;
static uint32_t rangeEnd = DEFAULT_END_MM;
static uint8_t currentProfile = DEFAULT_MAX_PROFILE;
static float smoothingFactor = DEFAULT_SMOOTHING;

// Statistiche
static uint32_t totalReadings = 0;
static uint32_t validReadings = 0;
static uint32_t errorReadings = 0;


// === INIZIALIZZAZIONE ===
bool initRadar() {
    Serial.println("🔧 Inizializzazione XM125...");
    
    // Reset variabili
    firstReading = true;
    rawDistance = 0;
    filteredDistance = 0;
    kalmanFiltered = 0;
    
    // Inizializza comunicazione I2C
    if (radarSensor.begin(RADAR_I2C_ADDRESS, Wire) != 1) {
        Serial.println("❌ Errore inizializzazione radar XM125!");
        radarReady = false;
        return false;
    }
    
    Serial.println("✅ Radar XM125 connesso");
    
    // Configura profilo
    radarSensor.setMaxProfile(currentProfile);
    Serial.printf("⚙️  Profilo radar: %d\n", currentProfile);
    
    // Avvia servizio distanza
    int32_t setupError = radarSensor.distanceBegin();
    if (setupError != 0) {
        Serial.printf("❌ Errore in distanceBegin: %d\n", setupError);
        radarReady = false;
        return false;
    }
    
    // Setup lettura distanza
    uint32_t distanceSetupError = radarSensor.distanceDetectorReadingSetup();
    if (distanceSetupError != 0) {
        Serial.printf("❌ Errore in distanceDetectorReadingSetup: %d\n", distanceSetupError);
        radarReady = false;
        return false;
    }
    
    Serial.println("✅ Configurazione XM125 completata");
    radarReady = true;
    return true;
}

// === FUNZIONI DI STATO ===
bool isRadarReady() {
    return radarReady;
}

bool isRadarValid() {
    return radarReady && (rawDistance > 0) && 
           (rawDistance >= rangeStart) && (rawDistance <= rangeEnd);
}

// === NUOVA FUNZIONE PRINCIPALE (per FreeRTOS) ===
RadarData getRadarData() {
    RadarData data;
    data.timestamp_ms = millis();
    
    if (!radarReady) {
        data.valid = false;
        data.distance_mm = 0;
        data.filtered_distance_mm = 0;
        data.strength = 0;
        data.num_peaks = 0;
        return data;
    }
    
    // Setup lettura (necessario prima di ogni misura)
    uint32_t setupError = radarSensor.distanceDetectorReadingSetup();
    if (setupError != 0) {
        Serial.printf("⚠️ Errore setup lettura: %d\n", setupError);
        data.valid = false;
        errorReadings++;
        return data;
    }
    
    // Leggi distanza peak 0
    uint32_t distancePeak = 0;
    radarSensor.getDistancePeak0Distance(distancePeak);
    
    totalReadings++;
    
    if (distancePeak > 0) {
        rawDistance = (float)distancePeak;
        
        // === FILTRAGGIO A 3 STADI ===
        
        // 1. Filtro Kalman su valore raw
        kalmanFiltered = kalmanFilter.updateEstimate(rawDistance);
        
        // 2. Aggancio dinamico per grandi variazioni
        float deltaD = abs(kalmanFiltered - filteredDistance);
        
        if (firstReading || deltaD > 400) {
            // Prima lettura o salto > 400mm - aggancio immediato
            filteredDistance = kalmanFiltered;
            firstReading = false;
            Serial.printf("[RADAR] Aggancio immediato: %.0fmm\n", filteredDistance);
        } else {
            // 3. Smoothing EMA per piccole variazioni
            filteredDistance = smoothingFactor * kalmanFiltered + 
                             (1.0f - smoothingFactor) * filteredDistance;
        }
        
        // Popola struttura dati
        data.distance_mm = rawDistance;
        data.filtered_distance_mm = filteredDistance;
        data.strength = 100.0f; // TODO: Implementare lettura strength reale
        data.num_peaks = 1;
        data.valid = isRadarValid();
        
        if (data.valid) {
            validReadings++;
        }
    } else {
        data.valid = false;
        data.distance_mm = 0;
        data.filtered_distance_mm = filteredDistance; // Mantieni ultimo valore
        data.strength = 0;
        data.num_peaks = 0;
        errorReadings++;
    }
    
    return data;
}

// === FUNZIONI DI LETTURA (compatibilità) ===
float getRadarDistance() {
    if (!radarReady) return 0;
    
    // Forza aggiornamento se dati vecchi
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate > 100) {
        getRadarData();  // Aggiorna tutti i valori
        lastUpdate = millis();
    }
    
    return rawDistance;
}

float getFilteredRadarDistance() {
    if (!radarReady) return 0;
    
    // Forza aggiornamento se dati vecchi
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate > 100) {
        getRadarData();  // Aggiorna tutti i valori
        lastUpdate = millis();
    }
    
    return filteredDistance;
}

float getRadarStrength() {
    if (!radarReady) return 0;
    
    // TODO: Implementare lettura strength reale dal sensore
    // Per ora ritorna un valore fisso se c'è un target
    return (rawDistance > 0) ? 100.0f : 0.0f;
}

// === CONFIGURAZIONE ===
void setRadarRange(uint32_t startMm, uint32_t endMm) {
    rangeStart = constrain(startMm, 100, 5000);
    rangeEnd = constrain(endMm, rangeStart + 100, 10000);
    
    Serial.printf("📏 Radar range impostato: %d - %d mm\n", rangeStart, rangeEnd);
    
    // TODO: Applicare configurazione al sensore tramite registri
    // Esempio (da verificare con documentazione):
    // radarSensor.writeRegister(START_REGISTER, rangeStart);
    // radarSensor.writeRegister(END_REGISTER, rangeEnd);
}

void setRadarProfile(uint8_t profile) {
    if (profile >= 1 && profile <= 5) {
        currentProfile = profile;
        
        if (radarReady) {
            radarSensor.setMaxProfile(profile);
            Serial.printf("📡 Profilo radar cambiato: %d\n", profile);
            
            // Potrebbe essere necessario re-inizializzare dopo cambio profilo
            // radarSensor.distanceBegin();
            // radarSensor.distanceDetectorReadingSetup();
        }
    } else {
        Serial.printf("⚠️ Profilo non valido: %d (range: 1-5)\n", profile);
    }
}

// === CONFIGURAZIONE FILTRI ===
void setKalmanParameters(float processNoise, float measureNoise, float initError) {
    // Ricrea il filtro con nuovi parametri
    kalmanFilter = SimpleKalmanFilter(processNoise, measureNoise, initError);
    Serial.printf("🔧 Kalman filter: P=%.1f M=%.1f E=%.3f\n", 
                  processNoise, measureNoise, initError);
}

void setSmoothingFactor(float factor) {
    smoothingFactor = constrain(factor, 0.0f, 1.0f);
    Serial.printf("🔧 Smoothing factor: %.2f\n", smoothingFactor);
}

float getSmoothingFactor() {
    return smoothingFactor;
}

void resetKalmanFilter() {
    kalmanFilter = SimpleKalmanFilter(20, 10, 0.1);  // Reset ai valori default
    firstReading = true;
    Serial.println("🔧 Filtro Kalman resettato");
}

// === DEBUG ===
void printRadarDebug() {
    if (!radarReady) {
        Serial.println("Radar: NOT READY");
        return;
    }
    
    RadarData data = getRadarData();
    
    Serial.printf("Radar Debug - Raw:%.0f Kalman:%.0f Filtered:%.0f Valid:%d\n",
                  rawDistance, kalmanFiltered, filteredDistance, data.valid);
    
    // Statistiche
    float successRate = (totalReadings > 0) ? 
                       (validReadings * 100.0f / totalReadings) : 0;
    
    Serial.printf("Stats - Total:%d Valid:%d Errors:%d Success:%.1f%%\n",
                  totalReadings, validReadings, errorReadings, successRate);
}

// === FUNZIONI AGGIUNTIVE PER COMPLETEZZA ===
void printRadarConfig() {
    Serial.println("\n=== Configurazione Radar ===");
    Serial.printf("Range: %d - %d mm\n", rangeStart, rangeEnd);
    Serial.printf("Profilo: %d\n", currentProfile);
    Serial.printf("Smoothing: %.2f\n", smoothingFactor);
    Serial.printf("I2C Address: 0x%02X\n", RADAR_I2C_ADDRESS);
    Serial.printf("Stato: %s\n", radarReady ? "READY" : "NOT READY");
    Serial.println("========================\n");
}

// Funzione helper per test
void testRadarConnection() {
    Serial.println("\n=== Test Connessione Radar ===");
    
    // Prova a leggere un valore
    if (radarReady) {
        for (int i = 0; i < 5; i++) {
            float dist = getRadarDistance();
            Serial.printf("Test %d: %.0f mm\n", i+1, dist);
            delay(100);
        }
    } else {
        Serial.println("❌ Radar non pronto!");
    }
    
    Serial.println("============================\n");
}
