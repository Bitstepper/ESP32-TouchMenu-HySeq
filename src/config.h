
/*******************************************************************************
 * System Configuration
 ******************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// === HARDWARE PINS ===
#define LCD_WIDTH   240
#define LCD_HEIGHT  320
#define LCD_CS      42
#define LCD_RST     39
#define LCD_DC      41
#define LCD_SCK     40
#define LCD_MOSI    45
#define LCD_BL      5

#define TP_SDA      1
#define TP_SCL      3

// === COLORS ===
#define COLOR_BLACK       0x0000
#define COLOR_WHITE       0xFFFF
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_YELLOW      0xFFE0
#define COLOR_ORANGE      0xFD20
#define COLOR_CYAN        0x07FF
#define COLOR_DARKGREY    0x7BEF
#define COLOR_LIGHTBLUE   0xAEDC
#define COLOR_DARKGREEN   0x03E0
#define COLOR_DARKBLUE    0x0010
#define COLOR_PURPLE      0x780F

// === UI CONFIGURATION ===
struct UITheme {
    uint16_t bgColor = COLOR_LIGHTBLUE;
    uint16_t menuBgColor = COLOR_DARKGREY;
    uint16_t menuActiveColor = COLOR_BLACK;
    uint16_t textColor = COLOR_YELLOW;
    uint16_t headerTextColor = COLOR_DARKBLUE;
    uint16_t buttonColor = COLOR_DARKGREY;
    uint16_t buttonTextColor = COLOR_WHITE;
    uint8_t headerTextSize = 3;
    uint8_t menuTextSize = 2;
    uint8_t statusTextSize = 1;
};

// === MENU LAYOUT ===
#define MENU_X_OFFSET   20
#define MENU_Y_START    60
#define MENU_SPACING    60
#define MENU_WIDTH      200
#define MENU_HEIGHT     50
#define BUTTON_BACK_X   MENU_X_OFFSET
#define BUTTON_OK_X     (BUTTON_BACK_X + 100 + 20)
#define BUTTON_Y_POS    260
#define BUTTON_WIDTH    100
#define BUTTON_HEIGHT   40

// === SYSTEM SETTINGS ===
#define SERVICE_PIN     "235711"
#define DEBOUNCE_DELAY  200
#define DOUBLE_TAP_TIME 500

// === DEBUG ===
#define DEBUG_MENU      1
#define DEBUG_TOUCH     0

#endif // CONFIG_H
