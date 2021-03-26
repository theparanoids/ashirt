// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef CONNECTIONPROPERTIES_H
#define CONNECTIONPROPERTIES_H

#include <QGridLayout>
#include <QLabel>
#include <QObject>
#include <QPushButton>
#include <QLineEdit>
#include <QWidget>
#include <QSpacerItem>

#include "components/connection_checker/connectionchecker.h"
#include "config/server_item.h"

class ConnectionProperties : public QWidget {
  Q_OBJECT

 public:
  ConnectionProperties(QWidget* parent = nullptr);
  ~ConnectionProperties();

 private:
  /// buildUi constructs the window structure.
  void buildUi();
  /// wireUi connects UI elements together
  void wireUi();

  /// resetForm sets all fields to the loaded serverItem
  void resetForm();

  /// saveServer records the currently loaded server's updates to
  void saveServer();

  /// sanitizeName forces a regular name out of the proposed value. Currently quotes are replaced
  /// with '' and empty strings are replaced with Connection-timestamp
  QString sanitizeName(QString proposed);

  /// isNameValid performs validation on a given string to see if it meets name rules.
  /// @returns true if proposedName has no flaws, false otherwise
  bool isNameValid(QString proposedName);

  void showBadNameNote();

 private slots:
  void onConnectionCheckerPressed();

 public:
  /// isDirty is used to track if the user has made changes.
  /// True indicates that unsaved data is present on this form
  bool isDirty();
  void clearForm();
  void setEnabled(bool enable);
  void highlightNameTextbox();

 signals:
  void serverChanged(ServerItem data);

 public slots:
  void loadItem(ServerItem item);
  void saveCurrentItem();


 private:
  ServerItem emptyItem;
  ServerItem loadedItem;
  QPalette normalBackground;
  QPalette errorBackground;

  // UI Elements
  QGridLayout* gridLayout = nullptr;

  QLabel* _nameLabel = nullptr;
  QLabel* _nameHelpLabel = nullptr;
  QLabel* _accessKeyLabel = nullptr;
  QLabel* _secretKeyLabel = nullptr;
  QLabel* _hostPathLabel = nullptr;

  ConnectionChecker* connectionStatus = nullptr;
  QLineEdit* nameTextBox = nullptr;
  QLineEdit* accessKeyTextBox = nullptr;
  QLineEdit* secretKeyTextBox = nullptr;
  QLineEdit* hostPathTextBox = nullptr;
  QSpacerItem* spacer = nullptr;
};

#endif // CONNECTIONPROPERTIES_H
