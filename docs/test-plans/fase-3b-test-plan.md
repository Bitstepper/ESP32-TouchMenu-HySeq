# 🧪 TEST PLAN FASE 3B - Sistema di Misura ESP32

**Versione**: 1.0  
**Data**: 16/06/2025  
**Fase Progetto**: 3A→3B Transition  
**Repository**: [ESP32-TouchMenu-HySeq](https://github.com/Bitstepper/ESP32-TouchMenu-HySeq)

---

## 🎯 OBIETTIVO FASE 3B

Validare l'integrazione completa del sistema attraverso test automatizzati e manuali per verificare:
- ⚡ Sincronizzazione sensori IMU↔Radar <10ms
- 🎮 Precisione touch interface >95%
- 📊 Stabilità display real-time 30Hz
- 🔧 Robustezza state machine
- 📈 Performance sotto stress

---

## 📋 SUITE DI TEST AUTOMATIZZATI

### Test 1: Sensor Synchronization ⭐ **CRITICAL**
**Durata**: 2-3 giorni  
**Target**: IMU↔Radar latency < 10ms  
**File**: `src/sync_test.h` (implementation esistente)

```cpp
class SyncTestSuite {
    void testSyncLatency() {
        // Acquisisce 100 campioni sincronizzati
        // Verifica timestamp alignment
        // Log: timestamp_imu, timestamp_radar, delta_ms
        // Criterio successo: 95%+ samples con delta < 10ms
    }
    
    void testSampleRate() {
        // Misura 50 campioni per verificare 10Hz target
        // Calcola intervalli e frequenza media
        // Criterio: 9.5-10.5 Hz range accettabile
    }
    
    void testBufferOverflow() {
        // Simula condizioni di overflow del buffer
        // Verifica gestione overrun e recovery
        // Max overflow acceptable: <10 in 1000 samples
    }
};
```

**Criteri di Successo**:
- ✅ Latenza sincronizzazione: <10ms (95% samples)
- ✅ Sample rate stability: 10.0 ±0.5 Hz
- ✅ Buffer overflow: <1% incidents
- ✅ Zero memory leaks durante test

### Test 2: Touch Precision & Responsiveness 🎮
**Durata**: 1 giorno  
**File Target**: `src/touch_handler.cpp`

```cpp
struct TouchTest {
    // Test coordinate accuracy
    void testButtonPrecision() {
        // 100 touch attempts per button area
        // Verifica hit rate >95% dentro boundaries
        // False positive rate <2%
    }
    
    // Test responsiveness  
    void testTouchLatency() {
        // Misura delay touch→action
        // Target: <50ms response time
    }
    
    // Test durante operazioni concurrent
    void testTouchDuringDataUpdate() {
        // Touch durante refresh display 30Hz
        // Verifica no perdita eventi touch
    }
};
```

**Criteri di Successo**:
- ✅ Button hit accuracy: >95%
- ✅ Touch response time: <50ms
- ✅ No touch loss durante display update
- ✅ Gestione multi-touch: disabled/ignored correttamente

### Test 3: Live Data Display Stability 📊
**Durata**: 1 giorno  
**File Target**: `src/leaf_actions.cpp`

```cpp
struct DisplayTest {
    void testRefreshRate() {
        // Monitor actual vs target 30Hz refresh
        // Misura frame drop rate
        // Verifica no flickering
    }
    
    void testDataConsistency() {
        // Verifica coherenza dati IMU↔Radar↔Display
        // No race conditions nei update
    }
    
    void testMemoryUsage() {
        // Monitor heap usage durante 1h continuous
        // Leak detection: <1KB/hour acceptable
    }
};
```

**Criteri di Successo**:
- ✅ Refresh rate: 30Hz ±2Hz stable
- ✅ Zero flickering visibile
- ✅ Data consistency: 100% coherent updates
- ✅ Memory leak: <1KB/hour

### Test 4: State Machine Robustness 🔧
**Durata**: 1-2 giorni  
**Coverage**: Tutti i menu transitions + error scenarios

```cpp
struct StateMachineTest {
    void testAllMenuPaths() {
        // Automated navigation attraverso tutti i menu
        // Verifica ogni transizione MAIN→SUB→LEAF→BACK
        // Include service PIN scenarios
    }
    
    void testErrorRecovery() {
        // Simula error conditions
        // Verifica graceful recovery su ogni stato
        // Timeout handling, sensor failures
    }
    
    void testEdgeCases() {
        // Multiple rapid touches
        // Service PIN retry limits
        // Navigation durante calibrazione
    }
};
```

**Criteri di Successo**:
- ✅ 100% menu paths navigabili
- ✅ Graceful error recovery
- ✅ No lockup states possibili
- ✅ Service PIN security: max 3 attempts

### Test 5: Performance Stress Testing 📈
**Durata**: 2-3 giorni (include 8h continuous run)
**Scope**: Sistema completo sotto carico

```cpp
struct StressTest {
    void testContinuousOperation() {
        // 8 ore operative continue
        // Monitor CPU, memoria, temperatura
        // Verifica stabilità long-term
    }
    
    void testHighFrequencyOperations() {
        // Rapid menu navigation
        // Fast calibration cycles
        // Burst data acquisition
    }
    
    void testResourceLimits() {
        // Memory pressure situations
        // I2C bus congestion
        // SPI display bandwidth limits
    }
};
```

**Criteri di Successo**:
- ✅ Uptime: 8+ ore senza reboot
- ✅ CPU usage: <50% media
- ✅ Memory stable: no degradation
- ✅ Temperature: dentro limiti ESP32

---

## 🔧 SETUP TEST ENVIRONMENT

### Hardware Requirements
```cpp
// Setup calibrato per testing
- Riferimento distanza: Metro laser ±0.5mm calibrato
- Piano inclinometrico: ±0.1° precision
- Multimetro: per monitoring alimentazione
- Oscilloscopio: per timing analysis (opzionale)
```

### Software Tools
```bash
# Test automation tools
- Arduino IDE 2.x con Serial Monitor enhanced
- Python scripts per data collection
- GitHub Actions per CI/CD (future)
- Performance monitoring dashboard
```

### Test Data Collection
```cpp
struct TestMetrics {
    // Timing metrics
    uint32_t boot_time_ms;              // Target: <3000ms
    uint32_t measure_latency_ms;        // Target: <100ms  
    uint32_t sync_delta_average_ms;     // Target: <5ms
    
    // Accuracy metrics
    float distance_error_mm;            // Target: ±2mm
    float pitch_error_degrees;          // Target: ±0.1°
    float yaw_error_degrees;            // Target: ±0.5°
    
    // Performance metrics
    float cpu_usage_percent;            // Target: <50%
    uint32_t free_heap_bytes;          // Monitor trend
    uint32_t longest_task_ms;          // Target: <20ms
};
```

---

## 📊 SUCCESS CRITERIA MATRIX

| Test Category | Metric | Target | Critical |
|---------------|--------|---------|----------|
| **Sync Timing** | IMU↔Radar latency | <10ms | ⭐ YES |
| **Accuracy** | Distance precision | ±2mm | ⭐ YES |
| **Accuracy** | Pitch precision | ±0.1° | ⭐ YES |
| **Accuracy** | Yaw precision | ±0.5° | YES |
| **Performance** | Boot time | <3000ms | YES |
| **Performance** | CPU usage | <50% avg | YES |
| **Stability** | 8h uptime | 100% | ⭐ YES |
| **UI** | Touch accuracy | >95% | ⭐ YES |
| **Display** | Refresh rate | 30Hz ±2Hz | YES |

**Legend**: ⭐ = Critico per avanzamento Fase 4

---

## 🤖 AUTOMATION STRATEGY

### Test Runner Implementation
```cpp
// File: src/test_runner.cpp
class TestRunner {
public:
    void runAllTests();
    void runCriticalTests();
    void generateReport();
    
private:
    TestMetrics collectMetrics();
    bool validateCriteria();
    void saveResults();
};
```

### Integration con Context Manager
```javascript
// Context Manager integration
function runAutomatedTests() {
    // Trigger test execution via serial commands
    // Monitor test progress via GitHub API
    // Auto-update test status in UI
}
```

### Reporting Framework
```markdown
# Test Report Template
## Executive Summary
- Tests Executed: [X/Y]
- Critical Tests Passed: [X/Y] 
- Overall Status: [PASS/FAIL/IN_PROGRESS]

## Detailed Results
[Auto-generated from TestRunner output]

## Performance Metrics
[Real-time data from test execution]

## Issues Discovered
[Auto-populated from failed tests]

## Recommendation
[GO/NO-GO for Phase 4]
```

---

## 📅 TIMELINE & MILESTONES

### Week 1 (Days 1-3): Critical Tests
- **Day 1**: Sensor Sync Testing + Fix issues
- **Day 2**: Touch Precision + Display Stability  
- **Day 3**: Performance Baseline + Critical fixes

### Week 2 (Days 4-7): Comprehensive Testing
- **Day 4-5**: State Machine + Edge Cases
- **Day 6**: 8-hour Stress Test execution
- **Day 7**: Results analysis + Report generation

### Decision Gate: Phase 3B→4
**Criteria**: All ⭐ Critical tests PASS
- **GO**: Proceed to Phase 4 UI finalization
- **NO-GO**: Address critical issues, repeat tests

---

## 🔗 INTEGRATION REFERENCES

### Implementation Files
- `src/sync_test.h` - Test suite classes
- `src/test_runner.cpp` - Test execution engine  
- `src/performance_monitor.h` - Metrics collection
- `docs/context-manager.html` - Test control UI

### Documentation Links
- [Hardware Setup Guide](../hardware/test-setup.md)
- [Performance Baselines](../metrics/performance-baseline.md)
- [Troubleshooting Guide](../support/test-troubleshooting.md)

### GitHub Integration
- Issues: Auto-create per failed tests
- Actions: Future CI/CD automation
- Releases: Tag successful test milestones

---

**Last Updated**: 16/06/2025  
**Next Review**: Post Phase 3B completion  
**Maintained by**: ESP32-TouchMenu-HySeq Development Team
