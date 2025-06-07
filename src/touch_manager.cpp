/*******************************************************************************
 * Menu Renderer Implementation
 ******************************************************************************/

#include "menu_renderer.h"

MenuRenderer::MenuRenderer() : highlightedIndex(-1), showBackButton(true), showOkButton(true) {
}

void MenuRenderer::drawMenu(MenuItem* menu, DisplayManager* display) {
    if (!menu || !display) return;
    
    // Clear screen
    display->clear();
    
    // Draw header
    drawHeader(menu->label, display);
    
    // Draw separator
    drawSeparator(55, display);
    
    // Draw menu items
    for (int i = 0; i < menu->children.size() && i < 3; i++) {
        drawMenuItem(i, menu->children[i]->label, 
                    (i == highlightedIndex), display);
    }
    
    // Draw buttons
    if (menu->parent != nullptr || showOkButton) {
        drawButtons(display);
    }
}

void MenuRenderer::drawHeader(const String& title, DisplayManager* display) {
    UITheme& theme = display->getTheme();
    
    // Draw header background
    display->fillRect(0, 0, LCD_WIDTH, 55, theme.bgColor);
    
    // Draw title
    display->setTextCentered(15, title, theme.headerTextSize, theme.headerTextColor);
    
    // Draw app name in smaller text
    display->setText(10, 40, "HySeq Plus", 1, theme.headerTextColor);
}

void MenuRenderer::drawMenuItem(int index, const String& label, bool selected, 
                               DisplayManager* display) {
    UITheme& theme = display->getTheme();
    
    int y = MENU_Y_START + (index * MENU_SPACING);
    uint16_t bgColor = selected ? theme.menuActiveColor : theme.menuBgColor;
    uint16_t textColor = selected ? COLOR_WHITE : theme.textColor;
    
    // Draw button background with rounded corners
    display->fillRoundRect(MENU_X_OFFSET, y, MENU_WIDTH, MENU_HEIGHT, 8, bgColor);
    display->drawRoundRect(MENU_X_OFFSET, y, MENU_WIDTH, MENU_HEIGHT, 8, COLOR_WHITE);
    
    // Center text in button
    int textY = y + (MENU_HEIGHT - 16) / 2;  // Assuming text height ~16 pixels
    display->setText(MENU_X_OFFSET + 20, textY, label, theme.menuTextSize, textColor);
}

void MenuRenderer::drawButtons(DisplayManager* display) {
    UITheme& theme = display->getTheme();
    
    // Back button
    if (showBackButton) {
        display->fillRoundRect(BUTTON_BACK_X, BUTTON_Y_POS, BUTTON_WIDTH, 
                              BUTTON_HEIGHT, 5, COLOR_ORANGE);
        display->drawRoundRect(BUTTON_BACK_X, BUTTON_Y_POS, BUTTON_WIDTH, 
                              BUTTON_HEIGHT, 5, COLOR_BLACK);
        display->setText(BUTTON_BACK_X + 25, BUTTON_Y_POS + 12, "BACK", 
                        2, COLOR_WHITE);
    }
    
    // OK/Confirm button
    if (showOkButton) {
        display->fillRoundRect(BUTTON_OK_X, BUTTON_Y_POS, BUTTON_WIDTH, 
                              BUTTON_HEIGHT, 5, COLOR_DARKGREEN);
        display->drawRoundRect(BUTTON_OK_X, BUTTON_Y_POS, BUTTON_WIDTH, 
                              BUTTON_HEIGHT, 5, COLOR_BLACK);
        display->setText(BUTTON_OK_X + 35, BUTTON_Y_POS + 12, "OK", 
                        2, COLOR_WHITE);
    }
}

void MenuRenderer::drawSeparator(int y, DisplayManager* display) {
    display->drawLine(0, y, LCD_WIDTH, y, COLOR_WHITE);
}

MenuAction MenuRenderer::handleTouch(TouchPoint p, MenuItem* menu) {
    MenuAction action = {ACTION_NONE, -1};
    
    if (!p.touched || !menu) return action;
    
    // Check menu items
    for (int i = 0; i < menu->children.size() && i < 3; i++) {
        int itemY = MENU_Y_START + (i * MENU_SPACING);
        if (isPointInRect(p.x, p.y, MENU_X_OFFSET, itemY, MENU_WIDTH, MENU_HEIGHT)) {
            highlightedIndex = i;
            action.type = ACTION_SELECT_ITEM;
            action.index = i;
            return action;
        }
    }
    
    // Check Back button
    if (showBackButton && menu->parent != nullptr) {
        if (isPointInRect(p.x, p.y, BUTTON_BACK_X, BUTTON_Y_POS, 
                         BUTTON_WIDTH, BUTTON_HEIGHT)) {
            action.type = ACTION_BACK;
            return action;
        }
    }
    
    // Check OK button
    if (showOkButton && highlightedIndex >= 0) {
        if (isPointInRect(p.x, p.y, BUTTON_OK_X, BUTTON_Y_POS, 
                         BUTTON_WIDTH, BUTTON_HEIGHT)) {
            action.type = ACTION_CONFIRM;
            action.index = highlightedIndex;
            return action;
        }
    }
    
    return action;
}

bool MenuRenderer::isPointInRect(int16_t px, int16_t py, 
                                 int16_t x, int16_t y, int16_t w, int16_t h) {
    return (px >= x && px <= x + w && py >= y && py <= y + h);
}
