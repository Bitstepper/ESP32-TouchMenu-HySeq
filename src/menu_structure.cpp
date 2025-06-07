/*******************************************************************************
 * Menu Structure Implementation
 ******************************************************************************/

#include "menu_structure.h"
#include "leaf_actions.h"
#include "service_menu.h"
#include "config.h"

MenuSystem::MenuSystem() : root(nullptr), currentMenu(nullptr), 
                          selectedIndex(-1), inLeafAction(false) {
}

MenuSystem::~MenuSystem() {
    // Clean up menu structure
    if (root) {
        // Recursive deletion would go here
        delete root;
    }
}

void MenuSystem::init() {
    buildMenuStructure();
    currentMenu = root;
    selectedIndex = -1;
}

void MenuSystem::buildMenuStructure() {
    // Create root menu
    root = new MenuItem("MAIN MENU", MENU_MAIN);
    
    // === MAIN MENU ITEMS ===
    mainMenuItems[0] = new MenuItem("MEASUREMENTS", MENU_SUB);
    mainMenuItems[1] = new MenuItem("CALIBRATION", MENU_SUB);  
    mainMenuItems[2] = new MenuItem("SERVICE MENU", MENU_SUB);
    mainMenuItems[2]->setRequiresPin(true);
    
    // Add to root
    for (int i = 0; i < 3; i++) {
        root->addChild(mainMenuItems[i]);
    }
    
    // === SUBMENU 1: MEASUREMENTS ===
    subMenuItems[0][0] = new MenuItem("Live Display");
    subMenuItems[0][0]->setAction(
        []() { LeafActions::showLiveData(); },
        []() {  /* Update gestito internamente in showLiveData */  }
    );
    
    subMenuItems[0][1] = new MenuItem("Data Logging");
    subMenuItems[0][1]->setAction(
		[]() { LeafActions::startDataAcquisition(); }
    );
    
    subMenuItems[0][2] = new MenuItem("Export Data");
    subMenuItems[0][2]->setAction(
        []() { LeafActions::exportCSV(); }
    );
    
    // Add to parent
    for (int i = 0; i < 3; i++) {
        mainMenuItems[0]->addChild(subMenuItems[0][i]);
    }
    
    // === SUBMENU 2: CALIBRATION ===
    subMenuItems[1][0] = new MenuItem("Gyro Calib");
    subMenuItems[1][0]->setAction(
        []() { LeafActions::calibrateGyro(); }
    );
    
    subMenuItems[1][1] = new MenuItem("Accel Calib");
    subMenuItems[1][1]->setAction(
        []() { LeafActions::calibrateAccel(); }
    );
    
    subMenuItems[1][2] = new MenuItem("Mag Calib");
    subMenuItems[1][2]->setAction(
        []() { LeafActions::calibrateMag(); }
    );
    
    for (int i = 0; i < 3; i++) {
        mainMenuItems[1]->addChild(subMenuItems[1][i]);
    }
    
    // === SUBMENU 3: SERVICE ===
    subMenuItems[2][0] = new MenuItem("Settings");
    subMenuItems[2][0]->setAction(
        []() { LeafActions::changeSettings(); }
    );
    
    subMenuItems[2][1] = new MenuItem("System Info");
    subMenuItems[2][1]->setAction(
        []() { LeafActions::showSystemInfo(); }
    );
    
    subMenuItems[2][2] = new MenuItem("Factory Reset");
    subMenuItems[2][2]->setAction(
        []() { LeafActions::factoryReset(); }
    );
    
    for (int i = 0; i < 3; i++) {
        mainMenuItems[2]->addChild(subMenuItems[2][i]);
    }
    
    #if DEBUG_MENU
    Serial.println("Menu structure built successfully");
    #endif
}

void MenuSystem::selectItem(int index) {
    if (index >= 0 && index < currentMenu->children.size()) {
        selectedIndex = index;
        #if DEBUG_MENU
        Serial.print("Selected item: ");
        Serial.println(currentMenu->children[index]->label);
        #endif
    }
}

bool MenuSystem::confirmSelection() {
    if (selectedIndex < 0 || selectedIndex >= currentMenu->children.size()) {
        return false;
    }
    
    MenuItem* selected = currentMenu->children[selectedIndex];
    
    // Check if PIN required
    if (selected->requiresPin) {
        if (!ServiceMenu::authenticate()) {
            return false;
        }
    }
    
    // Navigate or execute
    if (selected->type == MENU_LEAF) {
        executeLeafAction(selected);
        return true;
    } else if (selected->type == MENU_SUB) {
        currentMenu = selected;
        selectedIndex = -1;
        return true;
    }
    
    return false;
}

void MenuSystem::goBack() {
    if (inLeafAction) {
        inLeafAction = false;
   // Le azioni si fermano automaticamente quando ritornano     LeafActions::stopCurrentAction();
    } else if (currentMenu->parent != nullptr) {
        currentMenu = currentMenu->parent;
        selectedIndex = -1;
    }
}

void MenuSystem::executeLeafAction(MenuItem* item) {
    if (item->action) {
        inLeafAction = true;
        item->action();
    }
}

void MenuSystem::updateLeafAction() {
    if (inLeafAction && selectedIndex >= 0) {
        MenuItem* current = currentMenu->children[selectedIndex];
        if (current->updateAction) {
            current->updateAction();
        }
    }
}
