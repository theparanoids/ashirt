// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef SERVER_SETTING_H
#define SERVER_SETTING_H

#include <QString>
#include <QVariant>

namespace model {
class ServerSetting {
 public:
  ServerSetting() = default;
  ServerSetting(QString name, QString slug) {
    this->activeOperationName = name;
    this->activeOperationSlug = slug;
  }

 public:
  friend QDataStream& operator<<(QDataStream& out, const model::ServerSetting& v) {
    out << v.activeOperationName << v.activeOperationSlug;
    return out;
  }

  friend QDataStream& operator>>(QDataStream& in, model::ServerSetting& v) {
    in >> v.activeOperationName;
    in >> v.activeOperationSlug;
    return in;
  }

 public:
  QString activeOperationSlug = "";
  QString activeOperationName = "";
};
}  // namespace model

//QDataStream& operator<<(QDataStream& out, const QMap<QString, model::ServerSetting>& v) {
//  int count = v.size();
//  out << count;
//  for (auto key : v.keys()) {
//    out << key;
//    out << v[key];
//  }

//  return out;
//}

//QDataStream& operator>>(QDataStream& in, QMap<QString, model::ServerSetting>& v) {
//  int count;
//  in >> count;

//  QString name;
//  model::ServerSetting serverSetting;
//  for( int i = 0; i < count; i++) {
//    in >> name;
//    in >> serverSetting;
//    v[name] = serverSetting;
//  }
//  return in;
//}


Q_DECLARE_METATYPE(model::ServerSetting);

#endif // SERVER_SETTING_H
