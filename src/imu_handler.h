// imu_handler.h
#ifndef IMU_HANDLER_H
#define IMU_HANDLER_H

#include <Arduino.h>

// === STRUTTURA DATI IMU ===
struct IMUData {
    float pitch;
    float yaw;
    float roll;
    bool valid;
    uint32_t timestamp;
};

// === FUNZIONI IMU PUBBLICHE ===
// Inizializzazione
bool initIMU();
bool isIMUReady();
bool isIMUCalibrated();

// Lettura dati completa (NUOVA - per FreeRTOS)
IMUData getIMUData();

// Lettura valori singoli (per compatibilità)
float getIMUPitch();
float getIMURoll();
float getIMUYaw();

// Calibrazione magnetometro
void startMagCalibration();
void finishMagCalibration();
bool isMagCalibrationInProgress();  // AGGIUNTA - utile per UI
float getMagCalibrationProgress();   // AGGIUNTA - per progress bar (0.0-1.0)
bool loadCalibrationFromEEPROM();
void saveCalibrationToEEPROM();

// Configurazione parametri filtro
void setDeadZones(float pitchDZ, float yawDZ);  // AGGIUNTA - per tuning runtime
void getDeadZones(float &pitchDZ, float &yawDZ); // AGGIUNTA

// Debug
void printIMUDebug();
void resetIMUFilters();  // AGGIUNTA - utile per test

// === COSTANTI CONFIGURAZIONE ===
// Questi erano hardcoded nel codice originale, meglio esporli
#define IMU_I2C_ADDR_6DOF  0x6A    // LSM6DSOX
#define IMU_I2C_ADDR_MAG   0x1C    // LIS3MDL
#define IMU_SAMPLE_RATE_HZ 52      // Da configurazione originale
#define IMU_MAG_RATE_HZ    20      // Da configurazione originale

// Valori di default per filtri (dal codice originale)
#define DEFAULT_YAW_DEAD_ZONE   0.15f
#define DEFAULT_PITCH_DEAD_ZONE 0.1f

#endif // IMU_HANDLER_H
