// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef WINDOW_H
#define WINDOW_H

#include <QSystemTrayIcon>
#include <QTimer>

#include "db/databaseconnection.h"
#include "dtos/operation.h"
#include "dtos/github_release.h"
#include "forms/credits/credits.h"
#include "forms/evidence/evidencemanager.h"
#include "forms/porting/porting_dialog.h"
#include "forms/settings/settings.h"
#include "helpers/screenshot.h"
#include "hotkeymanager.h"
#include "tools/UGlobalHotkey/uglobalhotkeys.h"
#include "forms/add_operation/createoperation.h"

#ifndef QT_NO_SYSTEMTRAYICON

#include <QDialog>

QT_BEGIN_NAMESPACE
class QAction;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QMenu;
class QPushButton;
class QSpinBox;
class QTextEdit;
QT_END_NAMESPACE

/**
 * @brief The MessageType enum specifies how to respond to a click on a tray message
 * @see openServicesPath
 */
enum MessageType {
  /// NO_ACTION indicates that nothing should happen on a tray message click
  NO_ACTION,
  /// UPGRADE indicates that the user should be taken to the releases page
  UPGRADE,
  /// OPEN_PATH indicates that the user should be taken to a file browser
  OPEN_PATH
};

class TrayManager : public QDialog {
  Q_OBJECT

 public:
  TrayManager(DatabaseConnection *);
  ~TrayManager();

 private:
  void buildUi();
  void wireUi();
  qint64 createNewEvidence(const QString& filepath, const QString& evidenceType);
  void spawnGetInfoWindow(qint64 evidenceID);
  void showNoOperationSetTrayMessage();
  void checkForUpdate();
  void cleanChooseOpSubmenu();
  void cleanChooseServerSubmenu();
  /// setTrayMessage mostly mirrors QSystemTrayIcon::showMessage, but adds the ability to set a
  /// message type,
  /// providing a mechanism to smartly route the click to an action.
  void setTrayMessage(MessageType type, const QString &title, const QString &message,
                      QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,
                      int millisecondsTimeoutHint = 10000);

 private slots:
  void onOperationListUpdated(bool success, const std::vector<dto::Operation> &operations);
  void onReleaseCheck(bool success, const std::vector<dto::GithubRelease> &releases);
  void onTrayMenuOpened();
  void onTrayMessageClicked();

 public slots:
  void onScreenshotCaptured(const QString &filepath);
  void setActiveOperationLabel();
  void onCodeblockCapture();
  void captureAreaActionTriggered();
  void captureWindowActionTriggered();
  void captureCodeblockActionTriggered();

 protected:
  void closeEvent(QCloseEvent *event) override;

 private:
  DatabaseConnection *db = nullptr;
  HotkeyManager *hotkeyManager = nullptr;
  Screenshot *screenshotTool = nullptr;
  QTimer *updateCheckTimer = nullptr;
  MessageType currentTrayMessage = NO_ACTION;

  /// openServicesPath is a variable to store where, on click, to open a path the next time a tray message is displayed
  QString openServicesPath = "";

  // Subwindows
  Settings *settingsWindow = nullptr;
  EvidenceManager *evidenceManagerWindow = nullptr;
  Credits *creditsWindow = nullptr;
  PortingDialog *importWindow = nullptr;
  PortingDialog *exportWindow = nullptr;


  CreateOperation *createOperationWindow = nullptr;

  // UI Elements
  QSystemTrayIcon *trayIcon = nullptr;
  QMenu *trayIconMenu = nullptr;

  QAction *quitAction = nullptr;
  QAction *currentOperationMenuAction = nullptr;
  QAction *captureScreenAreaAction = nullptr;
  QAction *captureWindowAction = nullptr;
  QAction *showEvidenceManagerAction = nullptr;
  QAction *showCreditsAction = nullptr;
  QAction *addCodeblockAction = nullptr;

  QMenu *importExportSubmenu = nullptr;
  QAction *exportAction = nullptr;
  QAction *importAction = nullptr;
  QAction *showSettingsAction = nullptr;

  QMenu *chooseOpSubmenu = nullptr;
  QAction *chooseOpStatusAction = nullptr;
  QAction *newOperationAction = nullptr;
  QAction *selectedOperationAction = nullptr;  // note: do not delete; for reference only
  std::vector<QAction *> allOperationActions;

  QMenu *chooseServerSubmenu = nullptr;
  std::vector<QAction *> allServerActions;
  QAction *selectedServerAction = nullptr;  // note: do not delete; for reference only
};

#endif  // QT_NO_SYSTEMTRAYICON

#endif
