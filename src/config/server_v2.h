// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef SERVER_V2_H
#define SERVER_V2_H

#include <QJsonArray>
#include <unordered_map>
#include <vector>

#include "server_set.h"
#include "helpers/jsonhelpers.h"
#include "helpers/file_helpers.h"
#include "config/server_item.h"

class ServersV2 : public ServerSet {
 public:
  ServersV2() = default;

 public:
  static QJsonObject serialize(ServersV2 manifest) {
    QJsonObject o;
    QJsonArray a;
    std::vector<ServerItem> serverList = manifest.getServers(true);
    for(auto item : serverList) {
      a.push_back(ServerItem::serialize(item));
    }
    o.insert("servers", a);
    return o;
  }

 public:
  void deserialize(QByteArray rawData, bool clearServers=true) {
    std::vector<ServerItem> servers;
    parseJSONItemV2(rawData, [this, clearServers](QJsonObject obj, QJsonParseError err) {
      if (err.error == QJsonParseError::NoError) {
        auto a = obj["servers"].toArray();

        if (clearServers) {
          entries.clear();
        }

        for (auto element : a) {
          auto s = ServerItem::deserialize(element.toObject());
          entries[s.getServerUuid()] = s;
        }
      }
    });
    dirty = true;
  }

  ServerItem getServerByUuid(const QString& uuid) override {
    auto position = entries.find(uuid);
    if (position == entries.end()) {
      return ServerItem();
    }
    return position->second;
  }

  std::vector<ServerItem> getServers(bool includeDeleted) override {
    static std::vector<ServerItem> fullServerList; // cache this list to avoid re-sorting a no-change list

    if (dirty) {
      fullServerList.clear();
      fullServerList.reserve(entries.size());
      for (auto serverEntry : entries) {
        fullServerList.push_back(serverEntry.second);
      }
      std::sort(fullServerList.begin(), fullServerList.end(), [](ServerItem a, ServerItem b){return a.getId() < b.getId();});

      dirty = false; // remove the dirty flag, since the item has now been re-sorted
    }

    if (!includeDeleted) {
      std::vector<ServerItem> serverList;
      serverList.reserve(fullServerList.size());
      std::copy_if(fullServerList.begin(), fullServerList.end(), std::back_inserter(serverList),
                   [](ServerItem item){return !item.deleted;});
      return serverList;
    }

    return fullServerList;
  }

  bool deleteServer(const QString& uuid, DeleteType deleteAction) override {
    auto position = entries.find(uuid);
    if (position == entries.end()) {
      return false;
    }
    if (deleteAction == PLAIN_DELETE) {
      position->second.deleted = true;
    }
    else if(deleteAction == RESTORE) {
      position->second.deleted = false;
    }
    else if (deleteAction == PERMANENT_DELETE) {
      entries.erase(position);
    }
    dirty = true;
    return dirty;
  }

  bool addServer(ServerItem item) override {
    if (item.isValid()) {
      auto knownServers = getServers(true);
      int newId = knownServers.size() > 0 ? knownServers.back().getId() + 1 : 1;

      auto itemCopy = ServerItem(newId, item.getServerUuid(),
          item.serverName, item.accessKey, item.secretKey, item.hostPath, item.deleted);

      entries[itemCopy.getServerUuid()] = itemCopy;
      dirty = true;
    }
    return dirty;
  }

  bool updateServer(ServerItem item) override {
    auto foundItem = entries.find(item.getServerUuid());
    if (foundItem != entries.end()) {
      // only changing fields that are editable -- other fields are ignored.
      foundItem->second.serverName = item.serverName;
      foundItem->second.accessKey = item.accessKey;
      foundItem->second.secretKey = item.secretKey;
      foundItem->second.hostPath = item.hostPath;

      dirty = true;
    }
    return dirty;
  }

  bool hasServer(const QString& uuid) override {
    return getServerByUuid(uuid).isValid();
  }

 protected:
  void parseConfigData(const QByteArray& data) override {
    deserialize(data);
  }

  QByteArray toFileEncoding() override {
    auto data = serialize(*this);
    QJsonDocument doc(data);
    return doc.toJson();
  }

  QString getLoadedPath() override {
    return loadedPath;
  }

  void setLoadedPath(QString path) override {
    this->loadedPath = path;
  }

 private:
  std::unordered_map<QString, ServerItem> entries;
  QString loadedPath;
  bool dirty = true;
};

#endif // SERVER_V2_H
