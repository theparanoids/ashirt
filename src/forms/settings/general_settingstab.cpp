// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "general_settingstab.h"

#include <QDir>
#include <QFile>
#include <QFileDialog>

#include "components/custom_keyseq_edit/singlestrokekeysequenceedit.h"


GeneralSettingsTab::GeneralSettingsTab(QWidget *parent) : QWidget(parent) {
  buildUi();
  wireUi();
}

GeneralSettingsTab::~GeneralSettingsTab() {
  delete _eviRepoLabel;
  delete _captureAreaCmdLabel;
  delete _captureAreaShortcutLabel;
  delete _captureWindowCmdLabel;
  delete _captureWindowShortcutLabel;
  delete _recordCodeblockShortcutLabel;

  delete eviRepoTextBox;
  delete captureAreaCmdTextBox;
  delete captureAreaShortcutTextBox;
  delete captureWindowCmdTextBox;
  delete captureWindowShortcutTextBox;
  delete recordCodeblockShortcutTextBox;
  delete eviRepoBrowseButton;

  delete gridLayout;
}

void GeneralSettingsTab::buildUi() {
  gridLayout = new QGridLayout(this);

  _eviRepoLabel = new QLabel("Evidence Repository", this);
  _captureAreaCmdLabel = new QLabel("Capture Area Command", this);
  _captureAreaShortcutLabel = new QLabel("Shortcut", this);
  _captureWindowCmdLabel = new QLabel("Capture Window Command", this);
  _captureWindowShortcutLabel = new QLabel("Shortcut", this);
  _recordCodeblockShortcutLabel = new QLabel("Record Codeblock Shortcut", this);

  eviRepoTextBox = new QLineEdit(this);
  captureAreaCmdTextBox = new QLineEdit(this);
  captureAreaShortcutTextBox = new SingleStrokeKeySequenceEdit(this);
  captureWindowCmdTextBox = new QLineEdit(this);
  captureWindowShortcutTextBox = new SingleStrokeKeySequenceEdit(this);
  recordCodeblockShortcutTextBox = new SingleStrokeKeySequenceEdit(this);
  eviRepoBrowseButton = new QPushButton("Browse", this);

  spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding);

  // Layout
  /*        0                 1           2             3
       +---------------+-------------+------------+-------------+
    0  | Evi Repo Lbl  |    [Evi Repo TB]         | browseBtn   |
       +---------------+-------------+------------+-------------+
    1  | Cap A Cmd Lbl | [CapACmdTB] | CapASh lbl | [CapASh TB] |
       +---------------+-------------+------------+-------------+
    2  | Cap W Cmd Lbl | [CapWCmdTB] | CapWSh lbl | [CapWSh TB] |
       +---------------+-------------+------------+-------------+
    3  | CodeblkSh Lbl | [Codeblk TB] | <none>    | <none>      |
       +---------------+-------------+------------+-------------+
    4  | Vertical Spacer                                        |
       +---------------+-------------+------------+-------------+
  */

  // row 0
  int row = 0;
  gridLayout->addWidget(_eviRepoLabel, row, 0);
  gridLayout->addWidget(eviRepoTextBox, row, 1, 1, 3);
  gridLayout->addWidget(eviRepoBrowseButton, row, 4);

  // row 1
  row++;
  gridLayout->addWidget(_captureAreaCmdLabel, row, 0);
  gridLayout->addWidget(captureAreaCmdTextBox, row, 1);
  gridLayout->addWidget(_captureAreaShortcutLabel, row, 2);
  gridLayout->addWidget(captureAreaShortcutTextBox, row, 3, 1, 2);

  // row 2
  row++;
  gridLayout->addWidget(_captureWindowCmdLabel, row, 0);
  gridLayout->addWidget(captureWindowCmdTextBox, row, 1);
  gridLayout->addWidget(_captureWindowShortcutLabel, row, 2);
  gridLayout->addWidget(captureWindowShortcutTextBox, row, 3, 1, 2);

  // row 3
  row++;
  gridLayout->addWidget(_recordCodeblockShortcutLabel, row, 0);
  gridLayout->addWidget(recordCodeblockShortcutTextBox, row, 1);

  // row 4
  row++;
  gridLayout->addItem(spacer, row, 0, 1, gridLayout->columnCount());

  this->setLayout(gridLayout);
}

void GeneralSettingsTab::wireUi() {
  connect(eviRepoBrowseButton, &QPushButton::clicked, this, &GeneralSettingsTab::onBrowseClicked);
}

void GeneralSettingsTab::setMargin(int width) {
  gridLayout->setContentsMargins(width, width, width, width);
}

void GeneralSettingsTab::setFocus() {
  eviRepoTextBox->setFocus();
}

void GeneralSettingsTab::onBrowseClicked() {
  auto browseStart = eviRepoTextBox->text();
  browseStart = QFile(browseStart).exists() ? browseStart : QDir::homePath();
  auto filename = QFileDialog::getExistingDirectory(this, tr("Select a project directory"),
                                                    browseStart, QFileDialog::ShowDirsOnly);
  if (filename != nullptr) {
    eviRepoTextBox->setText(QDir::toNativeSeparators(filename));
  }
}

void GeneralSettingsTab::resetForm(GeneralSettingsStruct values) {
  eviRepoTextBox->setText(QDir::toNativeSeparators(values.evidenceRepo));
  captureAreaCmdTextBox->setText(values.captureAreaCmd);
  captureAreaShortcutTextBox->setKeySequence(QKeySequence::fromString(values.captureAreaShortcut));

  captureWindowCmdTextBox->setText(values.captureWindowCmd);
  captureWindowShortcutTextBox->setKeySequence(QKeySequence::fromString(values.captureWindowShortcut));
  recordCodeblockShortcutTextBox->setKeySequence(QKeySequence::fromString(values.captureCodeblockShortcut));
}

GeneralSettingsStruct GeneralSettingsTab::encodeForm() {
  GeneralSettingsStruct values;

  values.evidenceRepo = QDir::fromNativeSeparators(eviRepoTextBox->text());
  values.captureAreaCmd = captureAreaCmdTextBox->text();
  values.captureAreaShortcut = captureAreaShortcutTextBox->keySequence().toString();

  values.captureWindowCmd = captureWindowCmdTextBox->text();
  values.captureWindowShortcut = captureWindowShortcutTextBox->keySequence().toString();
  values.captureCodeblockShortcut = recordCodeblockShortcutTextBox->keySequence().toString();

  return values;
}
