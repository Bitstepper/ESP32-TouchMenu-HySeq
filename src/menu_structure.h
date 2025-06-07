/*******************************************************************************
 * Menu Structure Definition
 ******************************************************************************/

#ifndef MENU_STRUCTURE_H
#define MENU_STRUCTURE_H

#include <Arduino.h>
#include <vector>
#include <functional>

// Menu item types
enum MenuType {
    MENU_MAIN,
    MENU_SUB,
    MENU_LEAF
};

// Action types for touch handling
enum ActionType {
    ACTION_NONE,
    ACTION_SELECT_ITEM,
    ACTION_CONFIRM,
    ACTION_BACK
};

// Menu action result
struct MenuAction {
    ActionType type;
    int index;
};

// Forward declaration
class MenuItem;

// Leaf action function prototype
typedef std::function<void(void)> LeafAction;
typedef std::function<void(void)> UpdateAction;

// Menu item class
class MenuItem {
public:
    String label;
    MenuType type;
    std::vector<MenuItem*> children;
    MenuItem* parent;
    LeafAction action;
    UpdateAction updateAction;
    bool requiresPin;
    int id;
    
    MenuItem(const String& lbl, MenuType t = MENU_SUB) 
        : label(lbl), type(t), parent(nullptr), action(nullptr), 
          updateAction(nullptr), requiresPin(false), id(0) {}
    
    void addChild(MenuItem* child) {
        children.push_back(child);
        child->parent = this;
    }
    
    void setAction(LeafAction act, UpdateAction upd = nullptr) {
        action = act;
        updateAction = upd;
        type = MENU_LEAF;
    }
    
    void setRequiresPin(bool req) {
        requiresPin = req;
    }
};

// Main menu system class
class MenuSystem {
private:
    MenuItem* root;
    MenuItem* currentMenu;
    int selectedIndex;
    bool inLeafAction;
    
    // Pre-defined menu structure
    MenuItem* mainMenuItems[3];
    MenuItem* subMenuItems[3][3];
    
public:
    MenuSystem();
    ~MenuSystem();
    
    void init();
    MenuItem* getCurrentMenu() { return currentMenu; }
    int getSelectedIndex() { return selectedIndex; }
    bool isInLeafAction() { return inLeafAction; }
    
    void selectItem(int index);
    bool confirmSelection();
    void goBack();
    void updateLeafAction();
    
private:
    void buildMenuStructure();
    void executeLeafAction(MenuItem* item);
};

#endif // MENU_STRUCTURE_H
