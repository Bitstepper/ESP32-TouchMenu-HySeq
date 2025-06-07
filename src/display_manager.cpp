/*******************************************************************************
 * Display Manager Implementation
 ******************************************************************************/

#include "display_manager.h"

DisplayManager::DisplayManager() : initialized(false) {
    bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI, -1);
    gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT);
}

DisplayManager::~DisplayManager() {
    if (gfx) delete gfx;
    if (bus) delete bus;
}

bool DisplayManager::begin() {
    // Backlight on
    pinMode(LCD_BL, OUTPUT);
    digitalWrite(LCD_BL, HIGH);
    
    // Initialize display
    if (!gfx->begin()) {
        Serial.println("Display init failed");
        return false;
    }
    
    // Clear screen
    gfx->fillScreen(theme.bgColor);
    
    initialized = true;
    return true;
}

void DisplayManager::clear() {
    if (!initialized) return;
    gfx->fillScreen(theme.bgColor);
}

void DisplayManager::fillScreen(uint16_t color) {
    if (!initialized) return;
    gfx->fillScreen(color);
}

void DisplayManager::setText(int16_t x, int16_t y, const String& text, 
                            uint8_t size, uint16_t color) {
    if (!initialized) return;
    
    gfx->setTextSize(size);
    gfx->setTextColor(color);
    gfx->setCursor(x, y);
    gfx->print(text);
}

void DisplayManager::setTextCentered(int16_t y, const String& text, 
                                   uint8_t size, uint16_t color) {
    if (!initialized) return;
    
    int16_t textWidth = getTextWidth(text, size);
    int16_t x = (LCD_WIDTH - textWidth) / 2;
    setText(x, y, text, size, color);
}

void DisplayManager::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, 
                             uint16_t color) {
    if (!initialized) return;
    gfx->drawRect(x, y, w, h, color);
}

void DisplayManager::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, 
                             uint16_t color) {
    if (!initialized) return;
    gfx->fillRect(x, y, w, h, color);
}

void DisplayManager::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, 
                             uint16_t color) {
    if (!initialized) return;
    gfx->drawLine(x0, y0, x1, y1, color);
}

void DisplayManager::drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, 
                                  int16_t r, uint16_t color) {
    if (!initialized) return;
    gfx->drawRoundRect(x, y, w, h, r, color);
}

void DisplayManager::fillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, 
                                  int16_t r, uint16_t color) {
    if (!initialized) return;
    gfx->fillRoundRect(x, y, w, h, r, color);
}

int16_t DisplayManager::getTextWidth(const String& text, uint8_t size) {
    // Approximate calculation (6 pixels per char at size 1)
    return text.length() * 6 * size;
}
