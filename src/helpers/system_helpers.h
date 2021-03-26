#ifndef SYSTEM_HELPERS_H
#define SYSTEM_HELPERS_H

#include <QString>
#include <QDir>

#include "appconfig.h"
#include "appsettings.h"

class SystemHelpers {

 public:

  /// Returns (and creates, if necessary) the path to where evidence should be stored (includes
  /// ending path separator)
  static QString pathToEvidence() {
    auto op = AppSettings::getInstance().operationSlug();
    auto root = AppConfig::getInstance().evidenceRepo() + "/";
    if (op != "") {
      root += op + "/";
    }

    QDir().mkpath(root);
    return root;
  }

};

#endif // SYSTEM_HELPERS_H
