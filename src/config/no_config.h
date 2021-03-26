#ifndef NO_CONFIG_H
#define NO_CONFIG_H

#include "config.h"

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

class NoConfig : public Config {

 public:
  QByteArray toJsonString() {
    QJsonObject obj;
    return QJsonDocument(obj).toJson();
  }

  int version() { return 0; }
  QString errorText() { return _errorText; }
  QString evidenceRepo() { return ""; }
  QString captureScreenAreaCmd() { return ""; }
  QString captureScreenAreaShortcut() { return ""; }
  QString captureScreenWindowCmd() { return ""; }
  QString captureScreenWindowShortcut() { return ""; }
  QString captureCodeblockShortcut() { return ""; }

  // interface methods (setters)
  void setErrorText(QString newVal) { _errorText = newVal; }
  void setEvidenceRepo(QString newVal) { Q_UNUSED(newVal); }
  void setCaptureScreenAreaCmd(QString newVal) { Q_UNUSED(newVal); }
  void setCaptureScreenAreaShortcut(QString newVal) { Q_UNUSED(newVal); }
  void setCaptureScreenWindowCmd(QString newVal) { Q_UNUSED(newVal); }
  void setCaptureScreenWindowShortcut(QString newVal) { Q_UNUSED(newVal); }
  void setCaptureCodeblockShortcut(QString newVal) { Q_UNUSED(newVal); }


 private:
  QString _errorText = "";
};

#endif // NO_CONFIG_H
