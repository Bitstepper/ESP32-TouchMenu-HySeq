#ifndef SYNC_TEST_H
#define SYNC_TEST_H

#include "sensor_tasks.h"
#include "sync_queue.h"

// === TEST SUITE SINCRONIZZAZIONE ===
class SyncTestSuite {
private:
    struct TestResult {
        bool passed;
        String testName;
        String details;
        uint32_t executionTime;
    };
    
    std::vector<TestResult> results;
    
public:
    // === TEST 1: Verifica Latenza Sincronizzazione ===
    void testSyncLatency() {
        TestResult result;
        result.testName = "Sync Latency Test";
        uint32_t startTime = millis();
        
        // Raccogli 100 campioni
        int validSamples = 0;
        uint32_t totalDelta = 0;
        uint32_t maxDelta = 0;
        
        for (int i = 0; i < 100; i++) {
            SensorData data;
            if (getSensorDataTimeout(data, 200)) {
                if (data.radar_valid && data.imu_valid) {
                    validSamples++;
                    totalDelta += data.sync_delta_ms;
                    maxDelta = max(maxDelta, data.sync_delta_ms);
                }
            }
            delay(100);  // 10Hz
        }
        
        result.executionTime = millis() - startTime;
        
        if (validSamples > 90) {  // 90% campioni validi
            float avgDelta = totalDelta / (float)validSamples;
            result.passed = (avgDelta < 10.0 && maxDelta < 15);
            result.details = String("Avg: ") + avgDelta + "ms, Max: " + maxDelta + "ms";
        } else {
            result.passed = false;
            result.details = "Insufficient valid samples: " + String(validSamples);
        }
        
        results.push_back(result);
    }
    
    // === TEST 2: Frequenza Campionamento ===
    void testSampleRate() {
        TestResult result;
        result.testName = "Sample Rate Test";
        uint32_t startTime = millis();
        
        // Misura tempo per 50 campioni
        uint32_t timestamps[50];
        int samplesReceived = 0;
        
        for (int i = 0; i < 50; i++) {
            SensorData data;
            if (getSensorDataTimeout(data, 150)) {
                timestamps[samplesReceived++] = data.timestamp_ms;
            }
        }
        
        result.executionTime = millis() - startTime;
        
        if (samplesReceived >= 45) {  // 90% ricevuti
            // Calcola intervalli
            float avgInterval = 0;
            for (int i = 1; i < samplesReceived; i++) {
                avgInterval += (timestamps[i] - timestamps[i-1]);
            }
            avgInterval /= (samplesReceived - 1);
            
            float sampleRate = 1000.0 / avgInterval;
            result.passed = (sampleRate >= 9.5 && sampleRate <= 10.5);
            result.details = String("Rate: ") + sampleRate + " Hz";
        } else {
            result.passed = false;
            result.details = "Lost samples: " + String(50 - samplesReceived);
        }
        
        results.push_back(result);
    }
    
    // === TEST 3: Buffer Overflow ===
    void testBufferOverflow() {
        TestResult result;
        result.testName = "Buffer Overflow Test";
        uint32_t startTime = millis();
        
        // Sospendi consumo per causare overflow
        delay(1000);  // Accumula 10 campioni
        
        // Verifica statistiche
        TaskStats stats;
        getSensorTaskStats(stats);
        uint32_t overflowsBefore = stats.queue_overflows;
        
        // Aspetta altri campioni
        delay(1000);
        
        getSensorTaskStats(stats);
        uint32_t overflowsAfter = stats.queue_overflows;
        
        result.executionTime = millis() - startTime;
        result.passed = (overflowsAfter - overflowsBefore) < 10;
        result.details = "Overflows: " + String(overflowsAfter - overflowsBefore);
        
        results.push_back(result);
    }
    
    // === TEST 4: Coordinate 3D ===
    void testCoordinateCalculation() {
        TestResult result;
        result.testName = "3D Coordinate Test";
        uint32_t startTime = millis();
        
        // Test casi noti
        struct TestCase {
            float dist, pitch, yaw;
            float expected_x, expected_y, expected_z;
        } testCases[] = {
            {1000, 0, 0,     0, 1000, 0},      // Dritto avanti
            {1000, 0, 90,    1000, 0, 0},      // 90° destra
            {1000, 45, 0,    0, 707, 707},     // 45° su
            {1000, -30, 180, 0, -866, -500}    // 30° giù, dietro
        };
        
        bool allPassed = true;
        String details = "";
        
        for (auto &tc : testCases) {
            SensorData data = {0};
            data.distance_mm = tc.dist;
            data.pitch_deg = tc.pitch;
            data.yaw_deg = tc.yaw;
            
            calculateCoordinates(data);
            
            float tolerance = 5.0;  // 5mm tolleranza
            bool xOk = abs(data.x_mm - tc.expected_x) < tolerance;
            bool yOk = abs(data.y_mm - tc.expected_y) < tolerance;
            bool zOk = abs(data.z_mm - tc.expected_z) < tolerance;
            
            if (!xOk || !yOk || !zOk) {
                allPassed = false;
                details += String("Failed at P:") + tc.pitch + " Y:" + tc.yaw + " ";
            }
        }
        
        result.executionTime = millis() - startTime;
        result.passed = allPassed;
        result.details = allPassed ? "All cases passed" : details;
        
        results.push_back(result);
    }
    
    // === ESEGUI TUTTI I TEST ===
    void runAllTests() {
        Serial.println("\n=== SYNC TEST SUITE ===");
        results.clear();
        
        testSyncLatency();
        delay(500);
        
        testSampleRate();
        delay(500);
        
        testBufferOverflow();
        delay(500);
        
        testCoordinateCalculation();
        
        printResults();
    }
    
    // === STAMPA RISULTATI ===
    void printResults() {
        Serial.println("\n=== TEST RESULTS ===");
        int passed = 0;
        int total = results.size();
        
        for (auto &r : results) {
            Serial.printf("[%s] %s - %s (%.1fs)\n", 
                r.passed ? "PASS" : "FAIL",
                r.testName.c_str(),
                r.details.c_str(),
                r.executionTime / 1000.0
            );
            if (r.passed) passed++;
        }
        
        Serial.printf("\n=== SUMMARY: %d/%d PASSED (%.0f%%) ===\n", 
            passed, total, (passed * 100.0) / total);
    }
};

// === FUNZIONE HELPER PER MENU SERVICE ===
void runSyncTests() {
    gfx->fillScreen(BLACK);
    gfx->setCursor(10, 50);
    gfx->setTextColor(WHITE);
    gfx->println("Running Tests...");
    
    SyncTestSuite testSuite;
    testSuite.runAllTests();
    
    gfx->setCursor(10, 100);
    gfx->println("Tests Complete!");
    gfx->setCursor(10, 120);
    gfx->println("Check Serial Monitor");
    
    delay(3000);
}

#endif // SYNC_TEST_H
