#ifndef CONFIGV1_H
#define CONFIGV1_H

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include "config.h"
#include "helpers/jsonhelpers.h"
#include "helpers/constants.h"

class ConfigV1 : public Config {

 public:
  void parseConfig(QByteArray data) {
    parseJSONItemV2(data, [this](QJsonObject obj, QJsonParseError err) {
      if (err.error != QJsonParseError::NoError) {
        setErrorText(err.errorString());
        return;
      }
      fromJson(this, obj);
    });
  }

 public:
  static ConfigV1* generateDefaultConfig() {
    auto c = new ConfigV1();

    c->_evidenceRepo = Constants::defaultEvidenceRepo();

#ifdef Q_OS_MACOS
    c->_screenshotExec = "screencapture -s %file";
    c->_captureWindowExec = "screencapture -w %file";
#endif

    return c;
  }

  /// fromJson is a small helper to create a new ConfigV1, then parse the given
  /// (presumably) json string. Roughly equivalent to auto c = new ConfigV1().parseConfig()
  static ConfigV1* fromJson(QByteArray data) {
    auto temp = new ConfigV1();
    temp->parseConfig(data);

    return temp;
  }

 public:
  //interface methods
  QJsonObject toJsonObject() {
    QJsonObject obj;
    obj.insert(_keyEvidenceRepo(), _evidenceRepo);
    obj.insert(_keyAccessKey(), accessKey());
    obj.insert(_keySecretKey(), secretKey());
    obj.insert(_keyAPIURL(), apiURL());
    obj.insert(_keyScreenshotCommand(), _screenshotExec);
    obj.insert(_keyScreenshotShortcut(), _screenshotShortcutCombo);
    obj.insert(_keyCaptureWindowCommand(), _captureWindowExec);
    obj.insert(_keyCaptureWindowShortcut(), _captureWindowShortcut);
    obj.insert(_keyCaptureCodeblockShortcut(), _captureCodeblockShortcut);
    return obj;
  }

  QByteArray toJsonString() {
    return QJsonDocument(toJsonObject()).toJson();
  }

  // interface methods (getters)
  int version() { return 1; }
  QString errorText() { return _errorText; }
  QString evidenceRepo() { return _evidenceRepo; }
  QString captureScreenAreaCmd() { return _screenshotExec; }
  QString captureScreenAreaShortcut() { return _screenshotShortcutCombo; }
  QString captureScreenWindowCmd() { return _captureWindowExec; }
  QString captureScreenWindowShortcut() { return _captureWindowShortcut; }
  QString captureCodeblockShortcut() { return _captureCodeblockShortcut; }
  /// @deprecated Use DatabaseConnection::accessKey()
  QString accessKey() { return _accessKey; }
  /// @deprecated Use DatabaseConnection::secretKey()
  QString secretKey() { return _secretKey; }
  /// @deprecated Use DatabaseConnection::hostPath()
  QString apiURL() { return _apiURL; }


  // interface methods (setters)
  void setErrorText(QString newVal) { _errorText = newVal; }
  void setEvidenceRepo(QString newVal) { _evidenceRepo = newVal; }
  void setCaptureScreenAreaCmd(QString newVal) { _screenshotExec = newVal; }
  void setCaptureScreenAreaShortcut(QString newVal) { _screenshotShortcutCombo = newVal; }
  void setCaptureScreenWindowCmd(QString newVal) { _captureWindowExec = newVal; }
  void setCaptureScreenWindowShortcut(QString newVal) { _captureWindowShortcut = newVal; }
  void setCaptureCodeblockShortcut(QString newVal) { _captureCodeblockShortcut = newVal; }
  /// @deprecated Use DatabaseConnection::updateServerDetails()
  void setAccessKey(QString newVal) { _accessKey = newVal; }
  /// @deprecated Use DatabaseConnection::updateServerDetails()
  void setSecretKey(QString newVal) { _secretKey = newVal; }
  /// @deprecated Use DatabaseConnection::updateServerDetails()
  void setApiURL(QString newVal) { _apiURL = newVal; }


 private:
  static void fromJson(ConfigV1* inst, QJsonObject obj) {
    inst->_apiURL = obj[_keyAPIURL()].toString();
    inst->_accessKey = obj[_keyAccessKey()].toString();
    inst->_secretKey = obj[_keySecretKey()].toString();
    inst->_evidenceRepo = obj[_keyEvidenceRepo()].toString();
    inst->_screenshotExec = obj[_keyScreenshotCommand()].toString();
    inst->_screenshotShortcutCombo = obj[_keyScreenshotShortcut()].toString();
    inst->_captureWindowExec = obj[_keyCaptureWindowCommand()].toString();
    inst->_captureWindowShortcut = obj[_keyCaptureWindowShortcut()].toString();
    inst->_captureCodeblockShortcut = obj[_keyCaptureCodeblockShortcut()].toString();
  }

  // Keys for json values
  static QString _keyEvidenceRepo() { return "evidenceRepo"; }
  static QString _keyAccessKey() { return "accessKey"; }
  static QString _keySecretKey() { return "secretKey"; }
  static QString _keyAPIURL() { return "apiURL"; }
  static QString _keyScreenshotCommand() { return "screenshotCommand"; }
  static QString _keyScreenshotShortcut() { return "screenshotShortcut"; }
  static QString _keyCaptureWindowCommand() { return "captureWindowExec"; }
  static QString _keyCaptureWindowShortcut() { return "captureWindowShortcut"; }
  static QString _keyCaptureCodeblockShortcut() { return "captureCodeblockShortcut"; }

 private:
  QString _evidenceRepo = "";
  QString _screenshotExec = "";
  QString _screenshotShortcutCombo = "";
  QString _captureWindowExec = "";
  QString _captureWindowShortcut = "";
  QString _captureCodeblockShortcut = "";
  QString _errorText = "";

  // deprecated fields
  QString _serverUuid = "";
  QString _serverName = "";
  QString _accessKey = "";
  QString _secretKey = "";
  QString _apiURL = "";


};

#endif  // CONFIGV1_H
