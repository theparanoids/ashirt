// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QAction>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QErrorMessage>
#include <QGridLayout>
#include <QTabWidget>

#include "general_settingstab.h"
#include "connections_settingstab.h"
#include "components/connection_checker/connectionchecker.h"
#include "db/databaseconnection.h"
#include "hotkeymanager.h"

/**
 * @brief The Settings class represents the settings dialog that displays when
 * a user chooses the "settings" option in the tray menu
 */
class Settings : public QDialog {
  Q_OBJECT

 public:
  /**
   * @brief Settings constructs the settings menu. UI will be built and wired.
   * @param hotkeyManager a handle to the HotkeyManager, so hotkeys may be updated upon saving.
   * @param parent
   */
  explicit Settings(DatabaseConnection* db, HotkeyManager* hotkeyManager, QWidget* parent = nullptr);
  ~Settings();

 private:
  /// buildUi creates the window structure.
  void buildUi();
  /// wireUi connects the components to each other.
  void wireUi();
  /// onSaveClicked handles the save operation. Used when the "Save" button is pressed.
  void onSaveClicked();
  /// onCancelClicked handles the discard operation. Used when the "Cancel" button is pressed.
  void onCancelClicked();

  /// saveConnectionsData saves changes made in the ConnectionsSettingsTab
  void saveConnectionsData();

  /// saveGeneralData saves changes made in the GeneralSettingsTab
  void saveGeneralData();

 public slots:
  /// showEvent extends the native showEvent handler. Restores the UI to system values.
  void showEvent(QShowEvent* evt) override;
  /// closeEvent extends the native closeEvent handler. Saves data prior to closing.
  void closeEvent(QCloseEvent* event) override;


 private:
  /// hotkeyManager is a (shared) reference to the HotkeyManager. Not to be deleted.
  HotkeyManager* hotkeyManager;
  /// db is a shared reference to the database. Do not delete
  DatabaseConnection* db;

  QAction* closeWindowAction = nullptr;

  // Ui Elements
  QGridLayout* gridLayout;
  QTabWidget* tabControl;

  GeneralSettingsTab* generalTab = nullptr;
  ConnectionsSettingsTab* connectionsTab = nullptr;

  QSpacerItem* spacer = nullptr;
  QDialogButtonBox* buttonBox = nullptr;

  QErrorMessage* couldNotSaveSettingsMsg = nullptr;
};

#endif  // SETTINGS_H
