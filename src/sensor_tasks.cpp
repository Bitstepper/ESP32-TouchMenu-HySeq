
#include "sensor_tasks.h"
#include "imu_handler.h"
#include "radar_handler.h"
#include <Wire.h>


// === FORWARD DECLARATIONS ===
static void handleBackgroundCalibration();  // Aggiungi questa dichiarazione


// === VARIABILI GLOBALI ===
QueueHandle_t sensorDataQueue = NULL;
TaskHandle_t sensorTaskHandle = NULL;
SemaphoreHandle_t i2cMutex = NULL;
SemaphoreHandle_t displayMutex = NULL;
SemaphoreHandle_t eepromMutex = NULL;

// Statistiche interne
static TaskStats taskStats = {0};
static bool calibrationInProgress = false;
static float calibrationProgress = 0.0;

// === TASK PRINCIPALE SENSORI ===
void sensorAcquisitionTask(void *pvParameters) {
    RTOS_LOG("Sensor task started on core %d", xPortGetCoreID());
    
    // Variabili locali task
    SensorData sensorData;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t lastRadarRead = 0;
    uint32_t lastImuRead = 0;
    
    taskStats.current_state = TASK_STATE_RUNNING;
    
    while (1) {
        // Timestamp comune
        uint32_t currentTime = millis();
        sensorData.timestamp_ms = currentTime;
        
        // === LETTURA RADAR (con mutex I2C) ===
        if (TAKE_MUTEX(i2cMutex, MS_TO_TICKS(10))) {
            sensorData.distance_mm = getRadarDistance();
            sensorData.filtered_distance_mm = getFilteredRadarDistance();
            sensorData.radar_valid = isRadarValid();
            sensorData.radar_timestamp = millis();
            lastRadarRead = sensorData.radar_timestamp;
            GIVE_MUTEX(i2cMutex);
        } else {
            RTOS_LOG("Failed to get I2C mutex for radar");
            sensorData.radar_valid = false;
        }
        
        // === LETTURA IMU (con mutex I2C) ===
        if (TAKE_MUTEX(i2cMutex, MS_TO_TICKS(10))) {
            IMUData imuData = getIMUData();
            sensorData.pitch_deg = imuData.pitch;
            sensorData.yaw_deg = imuData.yaw;
            sensorData.roll_deg = imuData.roll;
            sensorData.imu_valid = imuData.valid;
            sensorData.imu_timestamp = millis();
            lastImuRead = sensorData.imu_timestamp;
            GIVE_MUTEX(i2cMutex);
        } else {
            RTOS_LOG("Failed to get I2C mutex for IMU");
            sensorData.imu_valid = false;
        }
        
        // === CALCOLO SINCRONIZZAZIONE ===
        sensorData.sync_delta_ms = abs((int32_t)sensorData.radar_timestamp - 
                                       (int32_t)sensorData.imu_timestamp);
        
        // Aggiorna statistiche
        taskStats.samples_acquired++;
        if (isSyncValid(sensorData)) {
            taskStats.sync_success++;
            // Media mobile per avg_sync_delta
            taskStats.avg_sync_delta_ms = (taskStats.avg_sync_delta_ms * 0.95) + 
                                         (sensorData.sync_delta_ms * 0.05);
        } else {
            taskStats.sync_failures++;
        }
        
        if (sensorData.sync_delta_ms > taskStats.max_sync_delta_ms) {
            taskStats.max_sync_delta_ms = sensorData.sync_delta_ms;
        }
        
        // === CALCOLO COORDINATE 3D ===
        if (sensorData.radar_valid && sensorData.imu_valid) {
            calculateCoordinates(sensorData);
        }
        
        // === INVIO A QUEUE ===
        if (xQueueSend(sensorDataQueue, &sensorData, 0) != pdTRUE) {
            // Queue piena, rimuovi il più vecchio
            SensorData dummy;
            xQueueReceive(sensorDataQueue, &dummy, 0);
            xQueueSend(sensorDataQueue, &sensorData, 0);
            taskStats.queue_overflows++;
            RTOS_LOG("Sensor queue overflow!");
        }
        
        // === GESTIONE CALIBRAZIONE ===
        if (calibrationInProgress) {
            // Logica calibrazione in background
            handleBackgroundCalibration();
        }
        
        // Attendi prossimo ciclo (10Hz)
        vTaskDelayUntil(&xLastWakeTime, MS_TO_TICKS(SENSOR_SAMPLE_RATE_MS));
    }
}

// === INIZIALIZZAZIONE ===
bool initSensorTasks() {
    // Crea mutex
    i2cMutex = xSemaphoreCreateMutex();
    displayMutex = xSemaphoreCreateMutex();
    eepromMutex = xSemaphoreCreateMutex();
    
    if (!i2cMutex || !displayMutex || !eepromMutex) {
        Serial.println("❌ Failed to create mutexes");
        return false;
    }
    
    // Crea queue sensori
    sensorDataQueue = xQueueCreate(SENSOR_QUEUE_SIZE, sizeof(SensorData));
    if (!sensorDataQueue) {
        Serial.println("❌ Failed to create sensor queue");
        return false;
    }
    
    // Crea task sensori
    BaseType_t result = xTaskCreatePinnedToCore(
        sensorAcquisitionTask,           // Funzione
        "SensorTask",                    // Nome
        SENSOR_TASK_STACK_SIZE,          // Stack size
        NULL,                            // Parametri
        SENSOR_TASK_PRIORITY,            // Priorità
        &sensorTaskHandle,               // Handle
        SENSOR_TASK_CORE                 // Core 1
    );
    
    if (result != pdPASS) {
        Serial.println("❌ Failed to create sensor task");
        return false;
    }
    
    Serial.println("✅ Sensor tasks initialized");
    return true;
}

// === CONTROLLO TASK ===
void suspendSensorTask() {
    if (sensorTaskHandle) {
        vTaskSuspend(sensorTaskHandle);
        taskStats.current_state = TASK_STATE_SUSPENDED;
    }
}

void resumeSensorTask() {
    if (sensorTaskHandle) {
        vTaskResume(sensorTaskHandle);
        taskStats.current_state = TASK_STATE_RUNNING;
    }
}

// === UTILITY ===
bool getLatestSensorData(SensorData &data) {
    // Non bloccante - prende l'ultimo disponibile
    return xQueuePeek(sensorDataQueue, &data, 0) == pdTRUE;
}

bool getSensorDataTimeout(SensorData &data, uint32_t timeout_ms) {
    return xQueueReceive(sensorDataQueue, &data, MS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void getSensorTaskStats(TaskStats &stats) {
    stats = taskStats;
}

void resetSensorTaskStats() {
    taskStats.samples_acquired = 0;
    taskStats.sync_success = 0;
    taskStats.sync_failures = 0;
    taskStats.queue_overflows = 0;
    taskStats.avg_sync_delta_ms = 0;
    taskStats.max_sync_delta_ms = 0;
}

// === CALIBRAZIONE ===
void startBackgroundCalibration() {
    calibrationInProgress = true;
    calibrationProgress = 0.0;
    taskStats.current_state = TASK_STATE_CALIBRATING;
}

bool isCalibrationComplete() {
    return !calibrationInProgress;
}

float getCalibrationProgress() {
    return calibrationProgress;
}

// Helper privato per calibrazione
static void handleBackgroundCalibration() {
    static uint32_t calibStartTime = 0;
    static int calibStep = 0;
    
    if (calibStep == 0) {
        calibStartTime = millis();
        calibStep = 1;
    }
    
    // Simula progresso calibrazione (20 secondi)
    uint32_t elapsed = millis() - calibStartTime;
    calibrationProgress = min(1.0f, elapsed / 20000.0f);
    
    if (elapsed >= 20000) {
        calibrationInProgress = false;
        calibrationProgress = 1.0;
        calibStep = 0;
        taskStats.current_state = TASK_STATE_RUNNING;
        RTOS_LOG("Calibration complete");
    }
}


