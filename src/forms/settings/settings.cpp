// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "settings.h"

#include <QFileDialog>
#include <QKeySequence>
#include <QString>
#include <algorithm>

#include "appconfig.h"
#include "appservers.h"
#include "helpers/ui_helpers.h"
#include "hotkeymanager.h"
#include "components/custom_keyseq_edit/singlestrokekeysequenceedit.h"

Settings::Settings(DatabaseConnection* db, HotkeyManager *hotkeyManager, QWidget *parent) : QDialog(parent) {
  this->db = db;
  this->hotkeyManager = hotkeyManager;
  buildUi();
  wireUi();
}

Settings::~Settings() {
  delete buttonBox;

  delete tabControl; // this will delete all tabs inside

  delete gridLayout;

  delete couldNotSaveSettingsMsg;
  delete closeWindowAction;
}

void Settings::buildUi() {
  gridLayout = new QGridLayout(this);
  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Save);
  buttonBox->addButton(QDialogButtonBox::Cancel);

  tabControl = new QTabWidget(this);
  generalTab = new GeneralSettingsTab();
  generalTab->setContentsMargins(0, 0, 0, 0);
  connectionsTab = new ConnectionsSettingsTab();
  connectionsTab->setContentsMargins(0, 0, 0, 0);

  couldNotSaveSettingsMsg = new QErrorMessage(this);


  // Layout
  /*        0                 1           2             3
       +---------------+-------------+------------+-------------+
    0  | Evi Repo Lbl  |    [Evi Repo TB]         | browseBtn   |
       +---------------+-------------+------------+-------------+
    1  | Cap A Cmd Lbl | [CapACmdTB] | CapASh lbl | [CapASh TB] |
       +---------------+-------------+------------+-------------+
    2  | Cap W Cmd Lbl | [CapWCmdTB] | CapWSh lbl | [CapWSh TB] |
       +---------------+-------------+------------+-------------+
    3  | CodeblkSh Lbl | [CodeblkSh TB] |                       |
       +---------------+-------------+------------+-------------+
    4  | Vertical spacer                                        |
       +---------------+-------------+------------+-------------+
    5  | Dialog button Box{save, cancel}                        |
       +---------------+-------------+------------+-------------+
  */

  tabControl->addTab(generalTab, "&General");
  tabControl->addTab(connectionsTab, "&Servers");

  // row 0
  int row = 0;
  gridLayout->addWidget(tabControl, row, 0);

  // row 1
  row++;
  gridLayout->addWidget(buttonBox, row, 0);

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  this->setLayout(gridLayout);
  this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  this->resize(760, 175);
  this->setWindowTitle("Settings");

  // Make the dialog pop up above any other windows but retain title bar and buttons
  Qt::WindowFlags flags = this->windowFlags();
  flags |= Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowMinMaxButtonsHint |
           Qt::WindowCloseButtonHint;
  this->setWindowFlags(flags);
}

void Settings::wireUi() {
  connect(buttonBox, &QDialogButtonBox::accepted, this, &Settings::onSaveClicked);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &Settings::onCancelClicked);
  connect(closeWindowAction, &QAction::triggered, this, &Settings::onSaveClicked);
}

void Settings::showEvent(QShowEvent *evt) {
  QDialog::showEvent(evt);
  auto& cfg = AppConfig::getInstance();

  GeneralSettingsStruct values;
  values.evidenceRepo = QDir::toNativeSeparators(cfg.evidenceRepo());
  values.captureAreaCmd = cfg.captureScreenAreaCmd();
  values.captureAreaShortcut = cfg.captureScreenAreaShortcut();
  values.captureWindowCmd = cfg.captureScreenWindowCmd();
  values.captureWindowShortcut = cfg.captureScreenWindowShortcut();
  values.captureCodeblockShortcut = cfg.captureCodeblockShortcut();

  generalTab->setFocus();
  generalTab->resetForm(values);  // reset the form in case a user left junk in the text boxes and pressed "cancel"
  connectionsTab->resetForm();
}

void Settings::closeEvent(QCloseEvent *event) {
  onSaveClicked();
  QDialog::closeEvent(event);
}

void Settings::onCancelClicked() {
  reject();
}

void Settings::onSaveClicked() {
  saveGeneralData();
  saveConnectionsData();

  close();
}

void Settings::saveConnectionsData() {
  // three step process:
  // 1. add/update servers we know from the mock list
  // 2. find servers we don't want from the known list
  // 3. delete servers we don't want from the known list

  QString nextServerUuid = "";
  bool limitToNewOnly = false;

  auto mockServers = connectionsTab->encodeServers();
  for (auto server : mockServers) {
    auto result = AppServers::getInstance().upsertServer(server);
    if (result == AppServers::Inserted) {
      nextServerUuid = server.getServerUuid();
      limitToNewOnly = true;
    }
    else if (result == AppServers::Updated && !limitToNewOnly) {
      nextServerUuid = server.getServerUuid();
    }
  }

  std::vector<QString> removeUuids;
  for (auto server : AppServers::getInstance().getServers()) {
    auto itr = std::find_if(mockServers.begin(), mockServers.end(), [&server](ServerItem item) {
      return item.getServerUuid() == server.getServerUuid();
    });

    if (itr == mockServers.end()) {
      removeUuids.push_back(server.getServerUuid());
    }
  }

  for(auto serverUuid : removeUuids) {
    AppServers::getInstance().purgeServer(serverUuid);
  }

  try {
    AppServers::getInstance().writeServers();
  }
  catch (std::exception &e) {
    couldNotSaveSettingsMsg->showMessage("Unable to save settings. Error: " + QString(e.what()));
  }

  if (nextServerUuid != "") {
    AppSettings::getInstance().setServerUuid(nextServerUuid);
  }
  else if (mockServers.size() == 1) {
    AppSettings::getInstance().setServerUuid(mockServers.at(0).getServerUuid());
  }
}

void Settings::saveGeneralData() {
  auto &cfg = AppConfig::getInstance();

  GeneralSettingsStruct values = generalTab->encodeForm();

  cfg.setEvidenceRepo(values.evidenceRepo);
  cfg.setCaptureScreenAreaCmd(values.captureAreaCmd);
  cfg.setCaptureScreenAreaShortcut(values.captureAreaShortcut);
  cfg.setCaptureScreenWindowCmd(values.captureWindowCmd);
  cfg.setCaptureScreenWindowShortcut(values.captureWindowShortcut);
  cfg.setCaptureCodeblockShortcut(values.captureCodeblockShortcut);

  try {
    AppConfig::getInstance().writeConfig();
  }
  catch (std::exception &e) {
    couldNotSaveSettingsMsg->showMessage("Unable to save settings. Error: " + QString(e.what()));
  }
  hotkeyManager->updateHotkeys();
}
