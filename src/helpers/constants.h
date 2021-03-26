#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QStandardPaths>
#include <QString>

#include <iostream>
#include <QRegularExpression>

class Constants {
 public:
  static QString unknownRepoValue() {
    return "???";
  }
  static QString unknownOwnerValue() {
    return "???";
  }

  static QString configLocation() {
#ifdef Q_OS_MACOS
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/config.json";
#else
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ashirt/config.json";
#endif
  }

  static QString serversLocation() {
#ifdef Q_OS_MACOS
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/servers.json";
#else
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/ashirt/servers.json";
#endif
  }

  static QString dbLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/evidence.sqlite";
  }

  static QString defaultEvidenceRepo() {
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/evidence";
  }

  static QString releaseOwner() {
    return parseRepo(RepoField::owner);
  }

  static QString releaseRepo() {
    return parseRepo(RepoField::repo);
  }

  static QString commitHash() {
    return QString("%1").arg(COMMIT_HASH);
  }

  static QString releaseTag() {
    static QString parsedReleaseTag = "";

    if (parsedReleaseTag.isEmpty()) {
      QRegularExpression compiledTagRegex("(?:tags/)?v(.*)");
      auto rawVersion = QString("%1").arg(VERSION_TAG);
      QRegularExpressionMatch match = compiledTagRegex.match(rawVersion);
      parsedReleaseTag = match.hasMatch() ? "v" + match.captured(1) : "v0.0.0-unversioned";
    }

    return parsedReleaseTag;
  }

  static QString userGuideUrl() {
    return "https://www.github.com/theparanoids/ashirt/blob/master/README.md";
  }

  static QString reportAnIssueUrl() {
    return "https://www.github.com/theparanoids/ashirt/issues";
  }

  static QString releasePageUrl() {
    return "https://github.com/theparanoids/ashirt/releases";
  }

  static QString codeFont() {
    #ifdef Q_OS_MACX
    return "monaco";
    #endif

    return "source code pro";
  }

  /// defaultDbName returns a string storing the "name" of the database for Qt identification
  /// purposes. This _value_ should not be reused for other db connections.
  static QString defaultDbName() {
    return "evidence";
  }
  
  static QString defaultServerName() { return "default"; }

  static QString legacyServerUuid() {
    return "20a28c7c-ea24-4ee0-bb94-0ee63018d34b"; // randomly generated value -- this needs to match the value in the multiple-backends migration
  }

 private:
  enum RepoField {
    owner = 0,
    repo = 1,
  };

  static QString parseRepo(RepoField field) {
    static QString parsedRepo = "";
    static QString parsedOwner = "";

    if (parsedRepo == "") {
      auto rawRepo = QString("%1").arg(SOURCE_CONTROL_REPO);
      QRegularExpression ownerRegex("^([^/]+)/(.*)");
      QRegularExpressionMatch match = ownerRegex.match(rawRepo);
      // Note that the specific values for the error cases below don't matter
      // They are set to avoid rerunning the parsing (since these values won't change mid-run)
      parsedOwner = match.hasMatch() ? match.captured(1) : unknownOwnerValue();
      parsedRepo = match.hasMatch() ? match.captured(2) : unknownRepoValue();
    }
    return field == RepoField::owner ? parsedOwner : parsedRepo;
  }
};

#endif // CONSTANTS_H
