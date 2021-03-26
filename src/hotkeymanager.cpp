// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "hotkeymanager.h"

#include <QString>
#include <iostream>

#include "appconfig.h"

HotkeyManager::HotkeyManager() {
  hotkeyManager = new UGlobalHotkeys();
  connect(hotkeyManager, &UGlobalHotkeys::activated, this, &HotkeyManager::hotkeyTriggered);
}

HotkeyManager::~HotkeyManager() { delete hotkeyManager; }

void HotkeyManager::registerKey(const QString& binding, GlobalHotkeyEvent evt) {
  hotkeyManager->registerHotkey(binding, size_t(evt));
}

void HotkeyManager::unregisterKey(GlobalHotkeyEvent evt) {
  hotkeyManager->unregisterHotkey(size_t(evt));
}

void HotkeyManager::hotkeyTriggered(size_t hotkeyIndex) {
  if (hotkeyIndex == ACTION_CAPTURE_AREA) {
    emit captureAreaHotkeyPressed();
  }
  else if (hotkeyIndex == ACTION_CAPTURE_WINDOW) {
    emit captureWindowHotkeyPressed();
  }
  else if (hotkeyIndex == ACTION_CAPTURE_CODEBLOCK) {
    emit codeblockHotkeyPressed();
  }
}

void HotkeyManager::updateHotkeys() {
  hotkeyManager->unregisterAllHotkeys();
  auto regKey = [this](QString combo, GlobalHotkeyEvent evt) {
    if (combo != "") {
      registerKey(combo, evt);
    }
  };
  regKey(AppConfig::getInstance().captureScreenAreaShortcut(), ACTION_CAPTURE_AREA);
  regKey(AppConfig::getInstance().captureScreenWindowShortcut(), ACTION_CAPTURE_WINDOW);
  regKey(AppConfig::getInstance().captureCodeblockShortcut(), ACTION_CAPTURE_CODEBLOCK);
}
