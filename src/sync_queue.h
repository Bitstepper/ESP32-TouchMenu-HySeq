#ifndef SYNC_QUEUE_H
#define SYNC_QUEUE_H

#include <Arduino.h>

// Struttura dati sincronizzata per sensori
struct SensorData {
    // Timestamp comune
    uint32_t timestamp_ms;
    
    // Dati Radar
    float distance_mm;
    float filtered_distance_mm;
    bool radar_valid;
    
    // Dati IMU
    float pitch_deg;
    float yaw_deg;
    float roll_deg;
    bool imu_valid;
    
    // Timestamp individuali per debug
    uint32_t radar_timestamp;
    uint32_t imu_timestamp;
    
    // Metriche sincronizzazione
    uint32_t sync_delta_ms;  // |radar_ts - imu_ts|
    
    // Coordinate 3D calcolate
    float x_mm;
    float y_mm;
    float z_mm;
};

// Configurazione Queue
#define SENSOR_QUEUE_SIZE 5      // Buffer di 5 misure
#define MAX_SYNC_DELTA_MS 10     // Max 10ms tra sensori

// Handle globale per la queue (definito in sensor_tasks.cpp)
extern QueueHandle_t sensorDataQueue;

// Funzioni helper
inline bool isSyncValid(const SensorData& data) {
    return data.sync_delta_ms <= MAX_SYNC_DELTA_MS;
}

// Calcola coordinate 3D da distanza e angoli
inline void calculateCoordinates(SensorData& data) {
    float dist_orizz = data.distance_mm * cos(data.pitch_deg * PI / 180.0);
    data.x_mm = dist_orizz * sin(data.yaw_deg * PI / 180.0);
    data.y_mm = dist_orizz * cos(data.yaw_deg * PI / 180.0);
    data.z_mm = data.distance_mm * sin(data.pitch_deg * PI / 180.0);
}

#endif // SYNC_QUEUE_H
