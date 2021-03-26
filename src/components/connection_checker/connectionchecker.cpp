#include "connectionchecker.h"

#include "helpers/netman.h"
#include "helpers/stopreply.h"
#include "dtos/checkConnection.h"
#include "helpers/http_status.h"


ConnectionChecker::ConnectionChecker(bool useAltLayout, QWidget *parent) : QWidget(parent) {
  buildUi(useAltLayout);
  wireUi();
}

ConnectionChecker::~ConnectionChecker() {
  stopReply(&currentTestReply);
  delete testConnButton;
  delete connStatusLabel;

  delete gridLayout;
}

void ConnectionChecker::buildUi(bool useAltLayout) {
  gridLayout = new QGridLayout(this);
  gridLayout->setContentsMargins(0, 0, 0, 0);

  testConnButton = new LoadingButton("Test Connection");
  testConnButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
  connStatusLabel = new QLabel(this);

  // Layout
  /*              0                           1
         +---------------------+--------------------------+
      0  |    Test Conn Btn    | Connection Status Label  |
         +---------------------+--------------------------+
     Alt
                   1
         +---------------------------+
      0  |    Test Conn Btn          |
         +---------------------------+
      1  |  Connection Status Label  |
         +---------------------------+
   */

  if (useAltLayout) {
    // row 0
    gridLayout->addWidget(testConnButton, 0, 0);
    // row 1
    gridLayout->addWidget(connStatusLabel, 1, 0);

  }
  else {
    // row 0
    gridLayout->addWidget(testConnButton, 0, 0);
    gridLayout->addWidget(connStatusLabel, 0, 1);
  }

  this->setLayout(gridLayout);
}

void ConnectionChecker::wireUi() {
  connect(testConnButton, &QPushButton::clicked, this, &ConnectionChecker::onTestConnectionClicked);
}

void ConnectionChecker::onTestConnectionClicked() {
  emit pressed();
  connStatusLabel->clear();
  if (hostPath.isEmpty()
      || accessKey.isEmpty()
      || secretKey.isEmpty()) {
    connStatusLabel->setText("Please set Access Key, Secret key and Host Path first.");
    return;
  }
  testConnButton->startAnimation();
  testConnButton->setEnabled(false);
  currentTestReply = NetMan::getInstance().testConnection(hostPath, accessKey, secretKey);
  connect(currentTestReply, &QNetworkReply::finished, this, &ConnectionChecker::onTestRequestComplete);
  emit started();
}

void ConnectionChecker::onTestRequestComplete() {
  bool ok = true;
  auto statusCode =
      currentTestReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(&ok);

  if (ok) {
    dto::CheckConnection connectionCheckResp;

    switch (statusCode) {
      case HttpStatus::StatusOK:
        connectionCheckResp = dto::CheckConnection::parseJson(currentTestReply->readAll());
        if (connectionCheckResp.parsedCorrectly && connectionCheckResp.ok) {
          connStatusLabel->setText("Connected");
        }
        else {
          connStatusLabel->setText("Unable to connect: Wrong or outdated server");
        }
        break;
      case HttpStatus::StatusUnauthorized:
        connStatusLabel->setText("Could not connect: Unauthorized (check access key and secret)");
        break;
      case HttpStatus::StatusNotFound:
        connStatusLabel->setText("Could not connect: Not Found (check URL)");
        break;
      default:
        QString msg = "Could not connect: Unexpected Error (code: %1)";
        connStatusLabel->setText(msg.arg(statusCode));
    }
  }
  else {
    connStatusLabel->setText(
        "Could not connect: Unexpected Error (check network connection and URL)");
  }

  testConnButton->stopAnimation();
  testConnButton->setEnabled(true);
  emit completed();
  tidyReply(&currentTestReply);
}

void ConnectionChecker::setConnectionTestFields(QString hostPath, QString accessKey, QString secretKey) {
  this->hostPath = hostPath;
  this->accessKey = accessKey;
  this->secretKey = secretKey;
}

void ConnectionChecker::clearStatus() {
  connStatusLabel->setText("");
}

void ConnectionChecker::abortRequest() {
  stopReply(&currentTestReply);
  clearStatus();
}
