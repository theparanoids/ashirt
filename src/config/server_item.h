// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef SERVER_ITEM_H
#define SERVER_ITEM_H

#include <QString>
#include <QUuid>
#include <QJsonObject>
#include <QVariant>
#include <QDataStream>

static QString newUuid() {
  return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

class ServerItem {
 public:
  ServerItem() = default;

  ServerItem(QString name, QString accessKey, QString secretKey, QString hostPath) :
     ServerItem(newUuid(), name, accessKey, secretKey, hostPath) {}
  ServerItem(QString uuid, QString name, QString accessKey, QString secretKey, QString hostPath) :
     ServerItem(0, uuid, name, accessKey, secretKey, hostPath, false) {}
  ServerItem(int id, QString uuid, QString name, QString accessKey, QString secretKey,
             QString hostPath, bool deleted) {
    this->id = id;
    this->serverName = name;
    this->accessKey = accessKey;
    this->secretKey = secretKey;
    this->hostPath = hostPath;
    this->deleted = deleted;
    this->uuid = uuid;
  }

 public:
  friend QDataStream& operator<<(QDataStream& out, const ServerItem& v) {
    auto copy = ServerItem(v);
    out << copy.getServerUuid() << copy.getId() << copy.serverName
        << copy.accessKey << copy.secretKey << copy.hostPath << copy.deleted;
    return out;
  }

  friend QDataStream& operator>>(QDataStream& in, ServerItem& v) {
    int id;
    QString uuid, name, accessKey, secretKey, hostPath;
    bool deleted;

    in >> uuid;
    in >> id;
    in >> name;
    in >> accessKey;
    in >> secretKey;
    in >> hostPath;
    in >> deleted;
    auto temp = ServerItem(id, uuid, name, accessKey, secretKey, hostPath, deleted);
    v = temp;
    return in;
  }

 public:
  static QJsonObject serialize(const ServerItem& item) {
    QJsonObject o;
    o.insert("id", item.id);
    o.insert("serverUuid", item.uuid);
    o.insert("serverName", item.serverName);
    o.insert("accessKey", item.accessKey);
    o.insert("secretKey", item.secretKey);
    o.insert("hostPath", item.hostPath);
    o.insert("deleted", item.deleted);
    return o;
  }
  static ServerItem deserialize(QJsonObject o) {
    ServerItem item;
    item.id = o.value("id").toInt();
    item.uuid = o.value("serverUuid").toString();
    item.serverName = o.value("serverName").toString();
    item.accessKey = o.value("accessKey").toString();
    item.secretKey = o.value("secretKey").toString();
    item.hostPath = o.value("hostPath").toString();
    item.deleted = o.value("deleted").toBool();
    return item;
  }

 public:
  bool isValid() { return uuid != ""; }
  QString getServerUuid() { return uuid; }
  int getId() { return id; }

  bool isEquivalentTo(ServerItem si) {
    return this->serverName == si.serverName &&
        this->hostPath == si.hostPath &&
        this->accessKey == si.accessKey &&
        this->secretKey == si.secretKey;
  }

  bool equals(ServerItem si) {
    return this->uuid == si.uuid && this->isEquivalentTo(si);
  }

 public:
  QString serverName = "";
  QString accessKey = "";
  QString secretKey = "";
  QString hostPath = "";
  bool deleted = false;

 private:
  /// uuid provides a mechanism for interacting with a specific server item
  /// it is critical that this value is preserved long-term
  QString uuid = "";
  /// id provides a mechanism to sort a server item collection -- this is similar to an auto incrementing id
  /// This value is allowed to change over runs of the application, if needed, or by accident
  int id = 0;
};

Q_DECLARE_METATYPE(ServerItem);
#endif // SERVER_ITEM_H
