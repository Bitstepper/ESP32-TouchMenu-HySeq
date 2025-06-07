// radar_handler.h
#ifndef RADAR_HANDLER_H
#define RADAR_HANDLER_H

#include <Arduino.h>

// === COSTANTI CONFIGURAZIONE RADAR ===
#define RADAR_I2C_ADDR         0x52    // Indirizzo I2C XM125
#define RADAR_DEFAULT_START_MM 250     // Range minimo default
#define RADAR_DEFAULT_END_MM   1500    // Range massimo default
#define RADAR_MAX_PEAKS        10      // Max target rilevabili
#define RADAR_SAMPLE_RATE_HZ   10      // Frequenza campionamento

// Profili radar (da documentazione XM125)
#define RADAR_PROFILE_1        1       // Ultra-short range
#define RADAR_PROFILE_2        2       // Short range (default)
#define RADAR_PROFILE_3        3       // Medium range
#define RADAR_PROFILE_4        4       // Long range
#define RADAR_PROFILE_5        5       // Max range

// Parametri filtro Kalman default
#define KALMAN_PROCESS_NOISE   20.0f   // Rumore processo
#define KALMAN_MEASURE_NOISE   10.0f   // Rumore misura  
#define KALMAN_INIT_ERROR      0.1f    // Errore iniziale

// === STRUTTURA DATI RADAR ===
struct RadarData {
    // Timestamp
    uint32_t timestamp_ms;
    
    // Distanza principale (peak 0)
    float distance_mm;
    float filtered_distance_mm;
    float strength;
    
    // Multi-target (opzionale)
    uint8_t num_peaks;
    float peak_distances[RADAR_MAX_PEAKS];
    float peak_strengths[RADAR_MAX_PEAKS];
    
    // Stato
    bool valid;
    bool near_start_edge;    // Oggetto vicino al limite minimo
    bool calibration_needed; // Radar necessita ricalibrazione
    
    // Temperatura sensore (per compensazione)
    int16_t temperature_c;
};

// === FUNZIONI RADAR PUBBLICHE ===
// Inizializzazione
bool initRadar();
bool isRadarReady();
bool isRadarValid();

// Lettura dati completa (NUOVA - per FreeRTOS)
RadarData getRadarData();

// Lettura valori singoli (compatibilità)
float getRadarDistance();         // Distanza raw
float getFilteredRadarDistance(); // Distanza filtrata
float getRadarStrength();         // Intensità segnale

// Multi-target
uint8_t getRadarNumPeaks();
bool getRadarPeak(uint8_t index, float &distance, float &strength);

// Configurazione range
void setRadarRange(uint32_t startMm, uint32_t endMm);
void getRadarRange(uint32_t &startMm, uint32_t &endMm);

// Configurazione profilo
void setRadarProfile(uint8_t profile);
uint8_t getRadarProfile();

// Configurazione filtro
void setKalmanParameters(float processNoise, float measureNoise, float initError);
void getKalmanParameters(float &processNoise, float &measureNoise);
void resetKalmanFilter();

// Configurazione smoothing
void setSmoothingFactor(float factor);  // 0.0-1.0 (0=no smooth, 1=max)
float getSmoothingFactor();

// Calibrazione radar
bool startRadarCalibration();
bool isRadarCalibrationComplete();
float getRadarCalibrationProgress();  // 0.0-1.0

// Statistiche
struct RadarStats {
    uint32_t total_readings;
    uint32_t valid_readings;
    uint32_t invalid_readings;
    float min_distance_seen;
    float max_distance_seen;
    float avg_strength;
    uint32_t calibration_count;
};

void getRadarStats(RadarStats &stats);
void resetRadarStats();

// Debug
void printRadarDebug();
void printRadarConfig();

// === CONFIGURAZIONI AVANZATE ===
// Soglie di rilevamento
void setDetectionThreshold(float threshold);
float getDetectionThreshold();

// Modalità peak sorting
enum PeakSortMode {
    SORT_BY_DISTANCE,    // Dal più vicino
    SORT_BY_STRENGTH     // Dal più forte
};
void setPeakSortMode(PeakSortMode mode);
PeakSortMode getPeakSortMode();

// Power management
void setRadarPowerMode(bool lowPower);
bool getRadarPowerMode();

#endif // RADAR_HANDLER_H
