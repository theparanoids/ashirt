// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "appservers.h"

#include "helpers/file_helpers.h"
#include "helpers/constants.h"
#include "config/server_v2.h"

QString AppServers::getServersFileLocation(const QString& path) {
  return path.isEmpty() ? Constants::serversLocation() : path;
}

void AppServers::loadServers(const QString& path) {
  auto filepath = getServersFileLocation(path);

  // eventually, figure out how to load the proper version

  serverSet = new ServersV2();
  serverSet->readFromServers(filepath);
}

ServerSet* AppServers::parseServers(const QByteArray& data) {
  serverSet = new ServersV2();
  serverSet->parseBytes(data);
  return serverSet;
}

bool AppServers::isLoadSuccessful() {
  return serverSet->isValid();
}

void AppServers::writeServers(const QString& path) {
  serverSet->writeToFile(getServersFileLocation(path));
}

void AppServers::addServer(const ServerItem& item) {
  if (serverSet->addServer(item)) {
    emit listUpdated();
  }
}

void AppServers::purgeServer(const QString& serverUuid) {
  if (serverSet->deleteServer(serverUuid, ServerSet::PERMANENT_DELETE)) {
    if (AppSettings::getInstance().serverUuid() == serverUuid) {
      AppSettings::getInstance().setServerUuid("");
    }
    emit listUpdated();
  }
}

void AppServers::deleteServer(const QString& serverUuid) {
  if (serverSet->deleteServer(serverUuid)) {
    if (AppSettings::getInstance().serverUuid() == serverUuid) {
      AppSettings::getInstance().setServerUuid("");
    }
    emit listUpdated();
  }
}

void AppServers::restoreServer(const QString& serverUuid) {
  if( serverSet->deleteServer(serverUuid, ServerSet::RESTORE) ) {
    emit listUpdated();
  }
}

ServerItem AppServers::getServerByUuid(const QString& serverUuid) {
  return serverSet->getServerByUuid(serverUuid);
}

std::vector<ServerItem> AppServers::getServers(bool includeDeleted) {
  return serverSet->getServers(includeDeleted);
}

void AppServers::updateServer(ServerItem item) {
  if( serverSet->updateServer(item) ) {
    emit listUpdated();
  }
}

ServerItem AppServers::selectServer(QString serverUuid) {
  return getServerByUuid(selectServerUuid(serverUuid));
}

QString AppServers::currentServerUuid() {
  return AppSettings::getInstance().serverUuid();
}

QString AppServers::selectServerUuid(QString maybeServerUuid) {
  return (maybeServerUuid == "") ? currentServerUuid() : maybeServerUuid;
}

AppServers::UpsertResult AppServers::upsertServer(ServerItem server) {
  if (serverSet->hasServer(server.getServerUuid())) {
    auto originalEntry = serverSet->getServerByUuid(server.getServerUuid());
    if (!originalEntry.equals(server)) {
      updateServer(server);
      return UpsertResult::Updated;
    }
    return UpsertResult::NoAction;
  }

  addServer(server);
  return UpsertResult::Inserted;
}

QString AppServers::accessKey(QString serverUuid) {
  return selectServer(serverUuid).accessKey;
}

QString AppServers::secretKey(QString serverUuid) {
  return selectServer(serverUuid).secretKey;
}

QString AppServers::hostPath(QString serverUuid) {
  return selectServer(serverUuid).hostPath;
}

QString AppServers::serverName(QString serverUuid) {
  return selectServer(serverUuid).serverName;
}
