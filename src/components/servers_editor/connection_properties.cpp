// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "connection_properties.h"

#include <QToolTip>
#include <QRegularExpression>

ConnectionProperties::ConnectionProperties(QWidget *parent) : QWidget(parent) {
  buildUi();
  wireUi();
}

ConnectionProperties::~ConnectionProperties() {
  delete _nameLabel;
  delete _accessKeyLabel;
  delete _secretKeyLabel;
  delete _hostPathLabel;

  delete connectionStatus;
  delete nameTextBox;
  delete accessKeyTextBox;
  delete secretKeyTextBox;
  delete hostPathTextBox;

  delete gridLayout;
}

void ConnectionProperties::buildUi() {
  gridLayout = new QGridLayout(this);
  gridLayout->setContentsMargins(0, 0, 0, 0);

  _nameLabel = new QLabel("Name");
  _nameHelpLabel = new QLabel(this);
  _nameHelpLabel->setPixmap(QPixmap(":/icons/help-icon-small.png"));
  _nameHelpLabel->setToolTip("Names must be non-empty and must not contain double quotes(\")");

  _hostPathLabel = new QLabel("Host Path");
  _accessKeyLabel = new QLabel("Access Key");
  _secretKeyLabel = new QLabel("Secret Key");
  _nameLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  _hostPathLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  _accessKeyLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  _secretKeyLabel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

  spacer = new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

  nameTextBox = new QLineEdit(this);
  hostPathTextBox = new QLineEdit(this);
  accessKeyTextBox = new QLineEdit(this);
  secretKeyTextBox = new QLineEdit(this);
  connectionStatus = new ConnectionChecker(true, this);
  connectionStatus->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

  normalBackground = nameTextBox->palette();
  errorBackground.setColor(QPalette::Base, QColor(0xFF, 0x99, 0x99));

  // Layout
  /*        0                 1            2
       +---------------+-------------+-------------+
    0  | Name Lbl      | [Name TB]   | help icon   |
       +---------------+-------------+-------------+
    1  | Host Lbl      | [Host TB]                 |
       +---------------+-------------+-------------+
    2  | Access Lbl    | [Access TB]               |
       +---------------+-------------+-------------+
    3  | Secret Lbl    | [Secret TB]               |
       +---------------+-------------+-------------+
    4  | Test Connection Area                      |
       +---------------+-------------+-------------+
    5  | vertical Spacer                           |
       +---------------+-------------+-------------+
  */

  // row 0
  int row = 0;
  gridLayout->addWidget(_nameLabel, row, 0);
  gridLayout->addWidget(nameTextBox, row, 1);
  gridLayout->addWidget(_nameHelpLabel, row, 2);

  // row 1
  row++;
  gridLayout->addWidget(_hostPathLabel, row, 0);
  gridLayout->addWidget(hostPathTextBox, row, 1, 1, 2);

  // row 2
  row++;
  gridLayout->addWidget(_accessKeyLabel, row, 0);
  gridLayout->addWidget(accessKeyTextBox, row, 1, 1, 2);

  // row 3
  row++;
  gridLayout->addWidget(_secretKeyLabel, row, 0);
  gridLayout->addWidget(secretKeyTextBox, row, 1, 1, 2);

  // row 4
  row++;
  gridLayout->addWidget(connectionStatus, row, 0, 1, gridLayout->columnCount(), Qt::AlignLeft);

  // row 5
  row++;
  gridLayout->addItem(spacer, row, 0, 1, gridLayout->columnCount());

  this->setLayout(gridLayout);
}

void ConnectionProperties::wireUi() {
  connect(connectionStatus, &ConnectionChecker::pressed, this, &ConnectionProperties::onConnectionCheckerPressed);
  connect(nameTextBox, &QLineEdit::textChanged, [this](QString value){
    nameTextBox->setPalette( isNameValid(value) ? normalBackground : errorBackground );
  });
}

void ConnectionProperties::saveServer() {
  QString nameText = sanitizeName(nameTextBox->text().trimmed());

  auto rtn = ServerItem(loadedItem.getId(), loadedItem.getServerUuid(), nameText,
                        accessKeyTextBox->text(), secretKeyTextBox->text(), hostPathTextBox->text(),
                        loadedItem.deleted);
  emit serverChanged(rtn);
}

bool ConnectionProperties::isNameValid(QString proposedName) {
  return proposedName.length() > 0 && !proposedName.contains("\"");
}

QString ConnectionProperties::sanitizeName(QString proposed) {
  auto revised = proposed.replace("\"", "''");
  if (revised.trimmed().length() == 0) {
    revised = QString("Connection-%1").arg(QDateTime::currentMSecsSinceEpoch());
  }
  return revised;
}

void ConnectionProperties::loadItem(ServerItem item) {
  bool isSameItem = loadedItem.getServerUuid() == item.getServerUuid();
  if (!isSameItem) {
    if (loadedItem.getServerUuid() != "") {
      saveServer();
    }
    nameTextBox->setFocus();
  }
  connectionStatus->abortRequest();
  loadedItem = item;
  resetForm();
}

void ConnectionProperties::saveCurrentItem() {
  if (loadedItem.getServerUuid() != "") {
    saveServer();
  }
}

void ConnectionProperties::resetForm() {
  nameTextBox->setText(loadedItem.serverName);
  nameTextBox->setPalette(normalBackground);
  accessKeyTextBox->setText(loadedItem.accessKey);
  secretKeyTextBox->setText(loadedItem.secretKey);
  hostPathTextBox->setText(loadedItem.hostPath);
  connectionStatus->clearStatus();
}

void ConnectionProperties::clearForm() {
  loadedItem = emptyItem;
  resetForm();
}

bool ConnectionProperties::isDirty() {
  return (loadedItem.serverName != nameTextBox->text()) ||
         (loadedItem.accessKey != accessKeyTextBox->text()) ||
         (loadedItem.secretKey != secretKeyTextBox->text()) ||
         (loadedItem.hostPath != hostPathTextBox->text());
}

void ConnectionProperties::onConnectionCheckerPressed() {
  auto hostPath = hostPathTextBox->text();
  auto accessKey = accessKeyTextBox->text();
  auto secretKey = secretKeyTextBox->text();

  connectionStatus->setConnectionTestFields(hostPath, accessKey, secretKey);
}

void ConnectionProperties::setEnabled(bool enable) {
  nameTextBox->setEnabled(enable);
  hostPathTextBox->setEnabled(enable);
  accessKeyTextBox->setEnabled(enable);
  secretKeyTextBox->setEnabled(enable);
  connectionStatus->setEnabled(enable);
}

void ConnectionProperties::highlightNameTextbox() {
  nameTextBox->selectAll();
}
