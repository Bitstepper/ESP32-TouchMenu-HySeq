/*******************************************************************************
 * Menu Renderer
 ******************************************************************************/

#ifndef MENU_RENDERER_H
#define MENU_RENDERER_H

#include "display_manager.h"
#include "menu_structure.h"
#include "touch_manager.h"

class MenuRenderer {
private:
    int highlightedIndex;
    bool showBackButton;
    bool showOkButton;
    
public:
    MenuRenderer();
    
    void drawMenu(MenuItem* menu, DisplayManager* display);
    MenuAction handleTouch(TouchPoint p, MenuItem* menu);
    
private:
    void drawHeader(const String& title, DisplayManager* display);
    void drawMenuItem(int index, const String& label, bool selected, 
                     DisplayManager* display);
    void drawButtons(DisplayManager* display);
    void drawSeparator(int y, DisplayManager* display);
    
    bool isPointInRect(int16_t px, int16_t py, 
                      int16_t x, int16_t y, int16_t w, int16_t h);
};

#endif // MENU_RENDERER_H
