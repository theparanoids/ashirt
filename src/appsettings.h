// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSequentialIterable>
#include <QSettings>
#include <QString>
#include <QMap>

#include "helpers/constants.h"
#include "models/tag.h"
#include "models/server_setting.h"

/// AppSettings is a singleton construct for accessing the application's settings. This is different
/// from configuration, as it represents the application's state, rather than how the application
/// communicates.
///
/// singleton design borrowed from:
/// https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
class AppSettings : public QObject {
  Q_OBJECT;

 public:
  static AppSettings &getInstance() {
    static AppSettings instance;
    return instance;
  }
  AppSettings(AppSettings const &) = delete;
  void operator=(AppSettings const &) = delete;

 private:
  AppSettings() : QObject(nullptr) {}

 public:
 signals:
  /// onOperationUpdated is signaled whenever the user chooses a new operation.
  void onOperationUpdated(QString operationSlug, QString operationName);
  /// onServerUpdated is signaled whenever the user changes servers
  void onServerUpdated(QString serverUuid);
  /// onSettingsSynced is signaled when the settings are written to disk (not guaranteed -- only called when the user asks to sync)
  void onSettingsSynced();

 public:
  /// sync tries to write the current settings to disk
  void sync() {
    settings.sync();  // ignoring the error
    emit this->onSettingsSynced();
  }

  /// upgrade migrates the settings from the current version to the newest version (or if at newest version, it does nothing)
  void upgrade() {
    int version = getVersion();
    if (version < 2) {
      upgradeToV2();
      setVersion(2);
    }
  }

 private:
  // internal fields
  /// getVersion retrieves the version of the settings data
  unsigned int getVersion() { return settings.value(settingVersion).toUInt(); }
  /// setVersion sets the version of the settings data (just the version number, not the actual format)
  /// @see upgrade
  void setVersion(unsigned int versionNumber) { settings.setValue(settingVersion, versionNumber); }

 private:
  /// upgradeToV2 migrates settings data from V1 to V2 (and no further)
  /// @see upgrade
  void upgradeToV2() {
    auto defaultServerUuid = Constants::legacyServerUuid();
    setServerUuid(defaultServerUuid);

    auto currentOpSlug = settings.value(opSlugSetting).toString();
    auto currentOpName = settings.value(opNameSetting).toString();

    if (currentOpName != "" && currentOpSlug != "") {
      updateServerSetting(defaultServerUuid, model::ServerSetting(currentOpName, currentOpSlug));
    }
    settings.remove(opSlugSetting);
    settings.remove(opNameSetting);
  }

 public:
  /// setOperationDetails saves the current operation slug and name, used throughout the application
  /// An onOperationUpdated signal is emitted after calling this method
  /// @see onOperationUpdated
  void setOperationDetails(QString opSlug, QString opName) {
    auto setting = getActiveServerSettings();
    setting.activeOperationName = opName;
    setting.activeOperationSlug = opSlug;
    updateActiveServerSetting(setting);

    emit onOperationUpdated(opSlug, opName);
  }
  /// operationSlug retrieves the currently "selected" operation slug
  QString operationSlug() { return getActiveServerSettings().activeOperationSlug; }
  /// operationName retrieves the currently "selected" operation name
  QString operationName() { return getActiveServerSettings().activeOperationName; }

  /// setLastUsedTags sets the tags used for future evidence creation
  void setLastUsedTags(std::vector<model::Tag> lastTags) {
    QVariantList writeTags;
    
    for (auto tag : lastTags) {
      writeTags << QVariant::fromValue(tag);
    }

    settings.setValue(lastUsedTagsSetting, QVariant::fromValue(writeTags));
  }
  
  /// getLastUsedTags retrieves the list of tags used for the previous evidence creation
  std::vector<model::Tag> getLastUsedTags() {
    std::vector<model::Tag> rtn;

    auto val = settings.value(lastUsedTagsSetting);

    if (val.canConvert<QVariantList>()) {
      QSequentialIterable iter = val.value<QSequentialIterable>();
      for (const QVariant& item : iter) {
        rtn.push_back(qvariant_cast<model::Tag>(item));
      }
    }

    return rtn;
  }

  /// getKnownServers retrieves the list of servers known to the application.  These servers are not
  /// used for connection info, but rather for metadata used within the application
  /// @see AppServers for connection info-related server data
  QMap<QString, model::ServerSetting> getKnownServers() {
    auto val = settings.value(knownServersSetting);
    return qvariant_cast<QMap<QString, model::ServerSetting>>(val);
  }

  /// setKnownServers updates the internal structure to use the given list as the list of known servers
  /// Note: not related to server connection info -- just metadata
  void setKnownServers(QMap<QString, model::ServerSetting> servers) {
    settings.setValue(knownServersSetting, QVariant::fromValue(servers));
  }

  /// updateServerSetting updates the server metadata with the given server uuid and setting data
  void updateServerSetting(QString serverUuid, model::ServerSetting newSetting, QString oldServerUuid="") {
    auto servers = getKnownServers();
    if (oldServerUuid != "") {
      removeServerSetting(oldServerUuid);
    }
    servers[serverUuid] = newSetting;
    setKnownServers(servers);
  }

  /// updateActiveServerSetting updates the settings (metadata) for the currently selected server.
  /// @see updateServerSetting
  void updateActiveServerSetting(model::ServerSetting newSetting) {
    updateServerSetting(serverUuid(), newSetting);
  }

  /// getActiveServerSettings retrieves the server (metadata) for the selected server. If no server is selected,
  /// then an empty settings object is returned
  model::ServerSetting getActiveServerSettings() {
    auto servers = getKnownServers();
    auto itr = servers.find(serverUuid());
    if (itr != servers.end()) {
      return itr.value();
    }
    return model::ServerSetting();
  }

  /// removeServerSetting deletes the settings data associated with the given server uuid
  bool removeServerSetting(QString serverUuid) {
    auto servers = getKnownServers();
    return servers.remove(serverUuid) != 0;
  }

  /// setServerUuid changes the current server settings with the server settings associated with the given uuid
  void setServerUuid(QString updatedServerUuid) {
    settings.setValue(activeServerSetting, updatedServerUuid);
    auto activeServerSetting = getActiveServerSettings();
    emit onOperationUpdated(activeServerSetting.activeOperationSlug, activeServerSetting.activeOperationName);
    emit onServerUpdated(updatedServerUuid);
  }

  /// serverUuid retrieves the currently selected server's uuid
  QString serverUuid() { return settings.value(activeServerSetting).toString(); }

 private:
  QSettings settings;

  // deprecated settings
  const char *opSlugSetting = "operation/slug"; // removed in v2 -- moved into server/known
  const char *opNameSetting = "operation/name"; // removed in v2 -- moved into server/known

  // active settings
  const char *settingVersion  = "settings/version";
  const char *activeServerSetting = "server/active";
  const char *knownServersSetting = "server/known";
  const char *lastUsedTagsSetting = "gather/tags";
};
#endif  // APPSETTINGS_H
