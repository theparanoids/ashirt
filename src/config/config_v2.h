#ifndef CONFIG_V2_H
#define CONFIG_V2_H

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include <unordered_map>

#include "basic_config.h"
#include "helpers/jsonhelpers.h"

class ConfigV2 : public BasicConfig {

 public:
  ConfigV2() {
    setVersion(2);
  }

 public:
  void parseConfig(QByteArray data) {
    parseJSONItemV2(data, [this](QJsonObject obj, QJsonParseError err) {
      BasicConfig::parseConfig(obj, err);

      _evidenceBase = obj[_keyEvidenceBase()].toString();
      _captureScreenAreaCmd = obj[_keyCaptureScreenAreaCmd()].toString();
      _captureScreenAreaCombo = obj[_keyCaptureScreenAreaCombo()].toString();
      _captureScreenWindowCmd = obj[_keyCaptureScreenWindowCmd()].toString();
      _captureScreenWindowCombo = obj[_keyCaptureScreenWindowCombo()].toString();
      _captureCodeblockCombo = obj[_keyCaptureCodeblockCombo()].toString();

    });
  }

 public:
  static ConfigV2* fromJson(QByteArray data) {
    auto temp = new ConfigV2();
    temp->parseConfig(data);

    return temp;
  }

  /// fromV1 will convert _any_ config into v2 format (though it is intended that a ConfigV1* is passed).
  /// If nullptr is passed instead, then an empty configv2 will be provided.
  /// Note: This will delete the passed object, and return a replacement.
  static ConfigV2* fromV1(Config* cfgv1) {
    auto rtn = new ConfigV2();

    if (cfgv1 != nullptr) {
      rtn->setEvidenceRepo(cfgv1->evidenceRepo());
      rtn->setCaptureScreenAreaCmd(cfgv1->captureScreenAreaCmd());
      rtn->setCaptureScreenAreaShortcut(cfgv1->captureScreenAreaShortcut());
      rtn->setCaptureScreenWindowCmd(cfgv1->captureScreenWindowCmd());
      rtn->setCaptureScreenWindowShortcut(cfgv1->captureScreenWindowShortcut());
      rtn->setCaptureCodeblockShortcut(cfgv1->captureCodeblockShortcut());

      delete cfgv1;
    }

    return rtn;
  }

 protected:
  QJsonObject buildJsonObject() {
    auto obj = BasicConfig::buildJsonObject();
    obj.insert(_keyEvidenceBase(), _evidenceBase);
    obj.insert(_keyCaptureScreenAreaCmd(), _captureScreenAreaCmd);
    obj.insert(_keyCaptureScreenAreaCombo(), _captureScreenAreaCombo);
    obj.insert(_keyCaptureScreenWindowCmd(), _captureScreenWindowCmd);
    obj.insert(_keyCaptureScreenWindowCombo(), _captureScreenWindowCombo);
    obj.insert(_keyCaptureCodeblockCombo(), _captureCodeblockCombo);

    return obj;
  }

 public: // interface methods
  QByteArray toJsonString() {
    return QJsonDocument(buildJsonObject()).toJson();
  };

  // interface methods (getters)
  QString evidenceRepo() { return _evidenceBase; }
  QString captureScreenAreaCmd() { return _captureScreenAreaCmd; }
  QString captureScreenAreaShortcut() { return _captureScreenAreaCombo; }
  QString captureScreenWindowCmd() { return _captureScreenWindowCmd; }
  QString captureScreenWindowShortcut() { return _captureScreenWindowCombo; }
  QString captureCodeblockShortcut() { return _captureCodeblockCombo; }

  // interface methods (setters)
  void setEvidenceRepo(QString newVal) { _evidenceBase = newVal; }
  void setCaptureScreenAreaCmd(QString newVal) { _captureScreenAreaCmd = newVal; }
  void setCaptureScreenAreaShortcut(QString newVal) { _captureScreenAreaCombo = newVal; }
  void setCaptureScreenWindowCmd(QString newVal) { _captureScreenWindowCmd = newVal; }
  void setCaptureScreenWindowShortcut(QString newVal) { _captureScreenWindowCombo = newVal; }
  void setCaptureCodeblockShortcut(QString newVal) { _captureCodeblockCombo = newVal; }

 private:

  // keys for json values
  static QString _keyEvidenceBase() { return "evidenceBase"; }
  static QString _keyCaptureScreenAreaCmd() { return "captureAreaCmd"; }
  static QString _keyCaptureScreenAreaCombo() { return "captureAreaShortcut"; }
  static QString _keyCaptureScreenWindowCmd() { return "captureWindowExec"; }
  static QString _keyCaptureScreenWindowCombo() { return "captureWindowShortcut"; }
  static QString _keyCaptureCodeblockCombo() { return "captureCodeblockShortcut"; }
  static QString _keyServers() { return "servers"; }

 private:
  QString _evidenceBase = "";
  QString _captureScreenAreaCmd = "";
  QString _captureScreenAreaCombo = "";
  QString _captureScreenWindowCmd = "";
  QString _captureScreenWindowCombo = "";
  QString _captureCodeblockCombo = "";

};

#endif // CONFIG_V2_H
