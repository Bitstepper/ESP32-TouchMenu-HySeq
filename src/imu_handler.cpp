// imu_handler.cpp
#include "imu_handler.h"
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_LIS3MDL.h>
#include <Adafruit_Sensor.h>

// Oggetti sensore
static Adafruit_LSM6DSOX lsm6ds;
static Adafruit_LIS3MDL lis3mdl;

// Variabili calibrazione
static float magOffset[3] = {0, 0, 0};
static float magScale[3] = {1, 1, 1};
static float minVal[3] = {10000, 10000, 10000};
static float maxVal[3] = {-10000, -10000, -10000};

// Variabili filtrate (GLOBALI per mantenere stato)
static float smoothPitch = 0;
static float smoothRoll = 0;
static float smoothYaw = 0;
static float smoothCos = 0;
static float smoothSin = 0;

// Valori raw correnti
static float currentPitch = 0;
static float currentRoll = 0;
static float currentYaw = 0;

// Zone morte (configurabili)
static float pitchDeadZone = DEFAULT_PITCH_DEAD_ZONE;
static float yawDeadZone = DEFAULT_YAW_DEAD_ZONE;

// Flag stato
static bool imuReady = false;
static bool calibrated = false;
static bool firstRun = true;

// Variabili calibrazione
static bool calibrationInProgress = false;
static float calibrationProgress = 0.0f;
static uint32_t calibrationStartTime = 0;

// === INIZIALIZZAZIONE ===
bool initIMU() {
    // Inizializza I2C su pin standard
    if (!lsm6ds.begin_I2C(IMU_I2C_ADDR_6DOF)) {
        Serial.println("❌ LSM6DSOX non trovato");
        return false;
    }
    
    if (!lis3mdl.begin_I2C(IMU_I2C_ADDR_MAG)) {
        Serial.println("❌ LIS3MDL non trovato");
        return false;
    }
    
    // Configurazione LSM6DSOX
    lsm6ds.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
    lsm6ds.setAccelDataRate(LSM6DS_RATE_52_HZ);
    lsm6ds.setGyroRange(LSM6DS_GYRO_RANGE_250_DPS);
    lsm6ds.setGyroDataRate(LSM6DS_RATE_52_HZ);
    
    // Configurazione LIS3MDL
    lis3mdl.setRange(LIS3MDL_RANGE_4_GAUSS);
    lis3mdl.setDataRate(LIS3MDL_DATARATE_20_HZ);
    lis3mdl.setPerformanceMode(LIS3MDL_HIGHMODE);
    
    Serial.println("✅ IMU inizializzata");
    
    // Carica calibrazione se presente
    calibrated = loadCalibrationFromEEPROM();
    imuReady = true;
    firstRun = true;
    
    return true;
}

// === NUOVA FUNZIONE PRINCIPALE PER FREERTOS ===
IMUData getIMUData() {
    IMUData data;
    data.timestamp = millis();
    
    if (!imuReady) {
        data.valid = false;
        data.pitch = 0.0f;
        data.yaw = 0.0f;
        data.roll = 0.0f;
        return data;
    }
    
    // Leggi tutti i sensori una volta sola
    sensors_event_t accel, gyro, temp, mag;
    lsm6ds.getEvent(&accel, &gyro, &temp);
    lis3mdl.getEvent(&mag);
    
    // === CALCOLO PITCH & ROLL ===
    float ax = accel.acceleration.x;
    float ay = accel.acceleration.y;
    float az = accel.acceleration.z;
    
    currentPitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
    currentRoll = atan2(ay, az) * 180.0 / PI;
    
    // === CALCOLO YAW CON COMPENSAZIONE TILT ===
    float pitchRad = currentPitch * DEG_TO_RAD;
    float rollRad = currentRoll * DEG_TO_RAD;
    
    // Applica calibrazione magnetometro
    float rawMag[3] = {mag.magnetic.x, mag.magnetic.y, mag.magnetic.z};
    float calibratedMag[3];
    for (int i = 0; i < 3; i++) {
        calibratedMag[i] = (rawMag[i] - magOffset[i]) / magScale[i];
    }
    
    // Compensazione tilt
    float mx = calibratedMag[0];
    float my = calibratedMag[1];
    float mz = calibratedMag[2];
    
    float mx2 = mx * cos(pitchRad) + mz * sin(pitchRad);
    float my2 = mx * sin(rollRad) * sin(pitchRad) + my * cos(rollRad) - mz * sin(rollRad) * cos(pitchRad);
    
    currentYaw = atan2(my2, mx2) * 180.0 / PI;
    if (currentYaw < 0) currentYaw += 360;
    
    // === APPLICAZIONE FILTRI ===
    if (firstRun) {
        smoothPitch = currentPitch;
        smoothRoll = currentRoll;
        smoothYaw = currentYaw;
        smoothCos = cos(currentYaw * DEG_TO_RAD);
        smoothSin = sin(currentYaw * DEG_TO_RAD);
        firstRun = false;
    } else {
        // Filtro Pitch con EMA e zona morta
        float deltaPitch = abs(currentPitch - smoothPitch);
        float alphaPitch = constrain(map(deltaPitch, 0, 10, 0.1, 0.3), 0.1, 0.3);
        float newPitch = alphaPitch * currentPitch + (1 - alphaPitch) * smoothPitch;
        
        if (abs(newPitch - smoothPitch) >= pitchDeadZone) {
            smoothPitch = newPitch;
        }
        
        // Filtro Roll (semplice EMA)
        smoothRoll = 0.2 * currentRoll + 0.8 * smoothRoll;
        
        // Filtro Yaw con approccio angolare
        float yawCos = cos(currentYaw * DEG_TO_RAD);
        float yawSin = sin(currentYaw * DEG_TO_RAD);
        
        float deltaYaw = abs(currentYaw - smoothYaw);
        if (deltaYaw > 180) deltaYaw = 360 - deltaYaw;
        
        // Zona morta adattiva basata su inclinazione
        float inclination = sqrt(currentPitch * currentPitch + currentRoll * currentRoll);
        float adaptiveYawDeadZone = constrain(
            map(inclination, 0, 90, yawDeadZone, 2.0), 
            yawDeadZone, 
            2.0
        );
        
        float alphaYaw = constrain(map(deltaYaw, 0, 10, 0.05, 0.3), 0.05, 0.3);
        
        float newCos = alphaYaw * yawCos + (1 - alphaYaw) * smoothCos;
        float newSin = alphaYaw * yawSin + (1 - alphaYaw) * smoothSin;
        
        float newYaw = atan2(newSin, newCos) * 180.0 / PI;
        if (newYaw < 0) newYaw += 360;
        
        if (abs(newYaw - smoothYaw) >= adaptiveYawDeadZone) {
            smoothYaw = newYaw;
            smoothCos = newCos;
            smoothSin = newSin;
        }
    }
    
    // Popola struttura dati
    data.pitch = smoothPitch;
    data.yaw = smoothYaw;
    data.roll = smoothRoll;
    data.valid = true;
    
    return data;
}

// === FUNZIONI COMPATIBILITÀ (ora usano i valori già calcolati) ===
float getIMUPitch() {
    if (!imuReady) return 0;
    
    // Se non abbiamo aggiornato di recente, forza un aggiornamento
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate > 100) {
        getIMUData();
        lastUpdate = millis();
    }
    
    return smoothPitch;
}

float getIMURoll() {
    if (!imuReady) return 0;
    
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate > 100) {
        getIMUData();
        lastUpdate = millis();
    }
    
    return smoothRoll;
}

float getIMUYaw() {
    if (!imuReady) return 0;
    
    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate > 100) {
        getIMUData();
        lastUpdate = millis();
    }
    
    return smoothYaw;
}

// === FUNZIONI STATO ===
bool isIMUReady() {
    return imuReady;
}

bool isIMUCalibrated() {
    return calibrated;
}

// === CALIBRAZIONE MAGNETOMETRO ===
void startMagCalibration() {
    Serial.println("🧲 Inizia calibrazione magnetometro...");
    calibrationInProgress = true;
    calibrationProgress = 0.0f;
    calibrationStartTime = millis();
    
    for (int i = 0; i < 3; i++) {
        minVal[i] = 10000;
        maxVal[i] = -10000;
    }
}

void finishMagCalibration() {
    if (!calibrationInProgress) return;
    
    for (int i = 0; i < 3; i++) {
        magOffset[i] = (maxVal[i] + minVal[i]) / 2.0;
        magScale[i] = (maxVal[i] - minVal[i]) / 2.0;
    }
    
    saveCalibrationToEEPROM();
    calibrated = true;
    calibrationInProgress = false;
    calibrationProgress = 1.0f;
    
    Serial.println("✅ Calibrazione completata");
}

bool isMagCalibrationInProgress() {
    return calibrationInProgress;
}

float getMagCalibrationProgress() {
    if (!calibrationInProgress) return calibrationProgress;
    
    // Aggiorna progresso basato sul tempo (20 secondi)
    uint32_t elapsed = millis() - calibrationStartTime;
    calibrationProgress = min(1.0f, elapsed / 20000.0f);
    
    // Durante la calibrazione, continua a leggere il magnetometro
    if (elapsed < 20000) {
        sensors_event_t mag;
        lis3mdl.getEvent(&mag);
        
        float raw[3] = {mag.magnetic.x, mag.magnetic.y, mag.magnetic.z};
        for (int i = 0; i < 3; i++) {
            minVal[i] = min(minVal[i], raw[i]);
            maxVal[i] = max(maxVal[i], raw[i]);
        }
    } else {
        // Auto-complete dopo 20 secondi
        finishMagCalibration();
    }
    
    return calibrationProgress;
}

// === EEPROM ===
bool loadCalibrationFromEEPROM() {
    EEPROM.begin(64);
    if (EEPROM.read(48) != 1) return false;
    
    for (int i = 0; i < 3; i++) {
        magOffset[i] = EEPROM.readFloat(i * 4);
        magScale[i] = EEPROM.readFloat((i + 3) * 4);
    }
    
    Serial.println("✅ Calibrazione caricata da EEPROM");
    return true;
}

void saveCalibrationToEEPROM() {
    EEPROM.begin(64);
    for (int i = 0; i < 3; i++) {
        EEPROM.writeFloat(i * 4, magOffset[i]);
        EEPROM.writeFloat((i + 3) * 4, magScale[i]);
    }
    EEPROM.write(48, 1);
    EEPROM.commit();
    Serial.println("✅ Calibrazione salvata in EEPROM");
}

// === CONFIGURAZIONE ===
void setDeadZones(float pitchDZ, float yawDZ) {
    pitchDeadZone = constrain(pitchDZ, 0.0f, 5.0f);
    yawDeadZone = constrain(yawDZ, 0.0f, 5.0f);
}

void getDeadZones(float &pitchDZ, float &yawDZ) {
    pitchDZ = pitchDeadZone;
    yawDZ = yawDeadZone;
}

// === DEBUG ===
void printIMUDebug() {
    IMUData data = getIMUData();
    Serial.printf("IMU Debug - P:%.1f Y:%.1f R:%.1f Valid:%d\n", 
        data.pitch, data.yaw, data.roll, data.valid);
}

void resetIMUFilters() {
    firstRun = true;
    smoothPitch = 0;
    smoothRoll = 0;
    smoothYaw = 0;
    smoothCos = 0;
    smoothSin = 0;
    Serial.println("✅ Filtri IMU resettati");
}
