/*******************************************************************************
 * Touch Manager
 ******************************************************************************/

#ifndef TOUCH_MANAGER_H
#define TOUCH_MANAGER_H

#include <Wire.h>
#include <CSE_CST328.h>
#include "config.h"

struct TouchPoint {
    int16_t x;
    int16_t y;
    bool touched;
    uint32_t timestamp;
};

class TouchManager {
private:
    CSE_CST328* touch;
    TwoWire* touchWire;
    bool initialized;
    TouchPoint lastTouch;
    uint32_t lastTouchTime;
    
public:
    TouchManager();
    ~TouchManager();
    
    bool begin();
    TouchPoint getTouch();
    bool isTouched();
    
    // Gesture detection
    bool isDoubleTap();
    bool isLongPress(uint32_t duration = 1000);
    
    // Calibration
    void calibrate();
    void saveCalibration();
    void loadCalibration();
    
private:
    void processTouch();
};

#endif // TOUCH_MANAGER_H
