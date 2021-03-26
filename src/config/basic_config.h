#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include "config.h"
#include "helpers/constants.h"
#include "helpers/jsonhelpers.h"

class BasicConfig : public Config {
 public:
  static int parseVersion(QByteArray data) {
    int version;
    std::string errorString;
    parseJSONItemV2(data, [&version, &errorString](QJsonObject obj, QJsonParseError err) {
      if (err.error != QJsonParseError::NoError) {
        errorString = err.errorString().toStdString();
        return;
      }
      version = obj[_keyVersion()].toInt();
    });

    if (errorString != "") {
      throw std::runtime_error("Unable to parse config file: " + errorString);
    }

    return version;
  }

 protected:
  void parseConfig(QJsonObject obj, QJsonParseError err) {
    if (err.error != QJsonParseError::NoError) {
      _errorText = err.errorString();
    }
    _parsedVersion = obj[_keyVersion()].toInt();
  }

  QJsonObject buildJsonObject() {
    QJsonObject obj;
    obj.insert(_keyVersion(), _parsedVersion);
    return obj;
  }

  void setVersion(int version) {
    _parsedVersion = version;
  }

 public:
  // interface methods
  virtual QByteArray toJsonString() = 0;

  // interface methods (getters)
  virtual int version() { return _parsedVersion; }
  virtual QString errorText() { return _errorText; }

  // interface methods (setters)
  virtual void setErrorText(QString newVal) { _errorText = newVal; }

 private:
  // keys for json values
  static QString _keyVersion() { return "version"; }

 private:
  int _parsedVersion = 0;
  QString _errorText = "";
};

#endif  // COMMON_CONFIG_H
