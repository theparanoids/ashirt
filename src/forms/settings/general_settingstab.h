// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef GENERALSETTINGSTAB_H
#define GENERALSETTINGSTAB_H

#include <QObject>
#include <QWidget>

#include <QDialogButtonBox>
#include <QErrorMessage>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpacerItem>
#include <QKeySequenceEdit>
#include <QSpacerItem>

class GeneralSettingsStruct {
 public:
  QString evidenceRepo;
  QString captureAreaCmd;
  QString captureAreaShortcut;
  QString captureWindowCmd;
  QString captureWindowShortcut;
  QString captureCodeblockShortcut;
};

class GeneralSettingsTab : public QWidget
{
  Q_OBJECT
 public:
  explicit GeneralSettingsTab(QWidget *parent = nullptr);
  ~GeneralSettingsTab();

 private:
  /// buildUi creates the window structure.
  void buildUi();
  /// wireUi connects the components to each other.
  void wireUi();

 private slots:
  /// onBrowseClicked triggers when the "browse" button is pressed. Shows a file dialog to the user.
  void onBrowseClicked();


 public:
  void resetForm(GeneralSettingsStruct values);
  GeneralSettingsStruct encodeForm();

  void setFocus();
  void setMargin(int width);

 private:
  // UI components
  QGridLayout* gridLayout = nullptr;
  QLabel* _eviRepoLabel = nullptr;
  QLabel* _captureAreaCmdLabel = nullptr;
  QLabel* _captureAreaShortcutLabel = nullptr;
  QLabel* _captureWindowCmdLabel = nullptr;
  QLabel* _captureWindowShortcutLabel = nullptr;
  QLabel* _recordCodeblockShortcutLabel = nullptr;

  QLineEdit* eviRepoTextBox = nullptr;
  QLineEdit* captureAreaCmdTextBox = nullptr;
  QKeySequenceEdit* captureAreaShortcutTextBox = nullptr;
  QLineEdit* captureWindowCmdTextBox = nullptr;
  QKeySequenceEdit* captureWindowShortcutTextBox = nullptr;
  QKeySequenceEdit* recordCodeblockShortcutTextBox = nullptr;
  QPushButton* eviRepoBrowseButton = nullptr;
  QSpacerItem* spacer = nullptr;
};

#endif // GENERALSETTINGSTAB_H
