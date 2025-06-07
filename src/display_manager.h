/*******************************************************************************
 * Display Manager
 ******************************************************************************/

#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino_GFX_Library.h>
#include "config.h"

class DisplayManager {
private:
    Arduino_DataBus* bus;
    Arduino_GFX* gfx;
    bool initialized;
    UITheme theme;
    
public:
    DisplayManager();
    ~DisplayManager();
    
    bool begin();
    void clear();
    void fillScreen(uint16_t color);
    
    // Text functions
    void setText(int16_t x, int16_t y, const String& text, 
                 uint8_t size = 1, uint16_t color = COLOR_WHITE);
    void setTextCentered(int16_t y, const String& text, 
                        uint8_t size = 1, uint16_t color = COLOR_WHITE);
    
    // Drawing functions
    void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
    void drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, 
                      int16_t r, uint16_t color);
    void fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, 
                      int16_t r, uint16_t color);
    
    // Theme functions
    void setTheme(const UITheme& newTheme) { theme = newTheme; }
    UITheme& getTheme() { return theme; }
    
    // Direct GFX access
    Arduino_GFX* getGfx() { return gfx; }
    
    // Utility
    int16_t width() { return LCD_WIDTH; }
    int16_t height() { return LCD_HEIGHT; }
    int16_t getTextWidth(const String& text, uint8_t size = 1);
};

#endif // DISPLAY_MANAGER_H
