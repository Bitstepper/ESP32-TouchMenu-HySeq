#ifndef SENSOR_TASKS_H
#define SENSOR_TASKS_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "task_config.h"
#include "sync_queue.h"

// === FUNZIONI TASK ===
// Task principale acquisizione sensori
void sensorAcquisitionTask(void *pvParameters);

// === INIZIALIZZAZIONE ===
// Inizializza task e risorse FreeRTOS
bool initSensorTasks();

// === CONTROLLO TASK ===
// Sospende/riprende il task sensori
void suspendSensorTask();
void resumeSensorTask();

// === UTILITY ===
// Ottiene l'ultimo dato sincronizzato (non bloccante)
bool getLatestSensorData(SensorData &data);

// Ottiene dato sincronizzato con timeout
bool getSensorDataTimeout(SensorData &data, uint32_t timeout_ms);

// Statistiche task
struct TaskStats {
    uint32_t samples_acquired;
    uint32_t sync_success;
    uint32_t sync_failures;
    uint32_t queue_overflows;
    float avg_sync_delta_ms;
    uint32_t max_sync_delta_ms;
    TaskState_t current_state;
};

void getSensorTaskStats(TaskStats &stats);
void resetSensorTaskStats();

// === CALIBRAZIONE ===
// Trigger calibrazione in background
void startBackgroundCalibration();
bool isCalibrationComplete();
float getCalibrationProgress();  // 0.0 - 1.0

#endif // SENSOR_TASKS_H
