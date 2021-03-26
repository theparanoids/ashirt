// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef APPSERVERS_H
#define APPSERVERS_H

#include <QObject>

#include "config/server_item.h"
#include "config/server_set.h"

class AppServers : public QObject {
  Q_OBJECT;

 public:
  enum UpsertResult {
    NoAction = 0,
    Inserted = 1,
    Updated = 2,
  };

  static AppServers &getInstance() {
    static AppServers instance;
    return instance;
  }
  AppServers(AppServers const &) = delete;
  void operator=(AppServers const &) = delete;

 private:
  AppServers() {
    loadServers();
  }

 private:
  QString getServersFileLocation(const QString& path);

  ServerItem selectServer(QString serverUuid="");
  QString selectServerUuid(QString maybeServerUuid);

 public:
  bool isLoadSuccessful();
  void loadServers(const QString& path="");
  ServerSet* parseServers(const QByteArray& data);

  /// writeServers attempts to write the in-memory server list to disk at the provided path.
  /// If an empty path is provided, then the write will be attempted at the standard location
  /// @see constants file
  void writeServers(const QString& path="");
  /// addServer creates a new server
  void addServer(const ServerItem& item);
  /// deleteServer marks the server associated with the given uuid as deleted
  void deleteServer(const QString& serverUuid);
  /// purgeServer actually deletes the server associated with given uuid
  void purgeServer(const QString& serverUuid);
  /// restoreServer marks the server associated with the given uuid as not-deleted (undoes deleteServer)
  void restoreServer(const QString& serverUuid);
  /// getServerByUuid retrieves the serer data associate with the given uuiid
  ServerItem getServerByUuid(const QString& serverUuid);
  /// getServers retrieves all of the servers known to the system. By default, only the undeleted servers
  /// are returned. Specify includeDeleted = true to return all servers, deleted or not
  std::vector<ServerItem> getServers(bool includeDeleted=false);
  /// updateServer updates the server provided. If the server does not exist, then nothing will be done.
  void updateServer(ServerItem item);

  /// upsertServer tries to either update a server (with the given uuid), or, if not present,
  /// adds the server instead. The resulting action is returned back to the caller
  /// @see UpsertResult
  UpsertResult upsertServer(ServerItem server);

  /// currentServerUuid retrieves the currently selected server's uuid
  QString currentServerUuid();
  /// accessKey is a shorthand for getting the accessKey for the given server uuid (or the current server, if the uuid is empty)
  QString accessKey(QString serverUuid="");
  /// secretKey is a shorthand for getting the secretKey for the given server uuid (or the current server, if the uuid is empty)
  QString secretKey(QString serverUuid="");
  /// hostPath is a shorthand for getting the hostPath for the given server uuid (or the current server, if the uuid is empty)
  QString hostPath(QString serverUuid="");
  /// serverName is a shorthand for getting the serverName for the given server uuid (or the current server, if the uuid is empty)
  QString serverName(QString serverUuid="");

 signals:
  /// listUpdated is signaled whenver a
  void listUpdated();


 private:
  ServerSet* serverSet;
};

#endif // APPSERVERS_H
