#ifndef TASK_CONFIG_H
#define TASK_CONFIG_H

// === CONFIGURAZIONI TASK ===
// Stack sizes (in words, not bytes!)
#define SENSOR_TASK_STACK_SIZE  4096    // 16KB per task sensori
#define DISPLAY_TASK_STACK_SIZE 8192    // 32KB per display (se futuro)

// Priorità task (0=lowest, configMAX_PRIORITIES-1=highest)
#define SENSOR_TASK_PRIORITY    2       // Media priorità
#define UI_TASK_PRIORITY       3       // Alta priorità per responsività
#define LOGGER_TASK_PRIORITY   1       // Bassa priorità

// Core assignment
#define SENSOR_TASK_CORE       1       // Core 1 dedicato ai sensori
#define UI_TASK_CORE          0       // Core 0 per UI/WiFi/BT

// Timing
#define SENSOR_SAMPLE_RATE_MS  100     // 10Hz come da specifica
#define UI_UPDATE_RATE_MS      33      // ~30Hz per display fluido
#define TOUCH_SCAN_RATE_MS     10      // 100Hz per touch responsivo

// === SEMAFORI E MUTEX ===
extern SemaphoreHandle_t i2cMutex;      // Protezione bus I2C
extern SemaphoreHandle_t displayMutex;  // Protezione display
extern SemaphoreHandle_t eepromMutex;   // Protezione EEPROM

// === TASK HANDLES ===
extern TaskHandle_t sensorTaskHandle;
extern TaskHandle_t displayTaskHandle;  // Per futuro uso

// === FLAGS DI STATO ===
typedef enum {
    TASK_STATE_INIT,
    TASK_STATE_RUNNING,
    TASK_STATE_CALIBRATING,
    TASK_STATE_ERROR,
    TASK_STATE_SUSPENDED
} TaskState_t;

// === MACRO UTILITY ===
#define TAKE_MUTEX(mutex, timeout) (xSemaphoreTake(mutex, timeout) == pdTRUE)
#define GIVE_MUTEX(mutex) xSemaphoreGive(mutex)

// Conversione ms to ticks
#define MS_TO_TICKS(ms) ((ms) / portTICK_PERIOD_MS)

// === DEBUG ===
#ifdef DEBUG_RTOS
  #define RTOS_LOG(fmt, ...) Serial.printf("[RTOS] " fmt "\n", ##__VA_ARGS__)
#else
  #define RTOS_LOG(fmt, ...)
#endif

#endif // TASK_CONFIG_H
