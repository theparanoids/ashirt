// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef JSONHELPERS_H
#define JSONHELPERS_H

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <functional>
#include <vector>
#include <functional>

// parseJSONList parses a JSON list into a vector of concrete types from a byte[]. If any error
// occurs during parsing, an empty vector is returned
template <typename T>
static std::vector<T> parseJSONList(QByteArray data, T (*dataToItem)(QJsonObject)) {
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError) {
    return std::vector<T>();
  }
  QJsonArray arr = doc.array();
  std::vector<T> list;

  for (QJsonValue val : arr) {
    auto item = dataToItem(val.toObject());
    list.push_back(item);
  }

  return list;
}

/// parseJSONItem parses a single item (assumed to be a Json Object) from a byte[]. If any error
/// occurs during parsing, an empty object is returned.
/// To inspect errors, use parseJSONItem(QByteArray, QJsonParseError)
template <typename T>
static T parseJSONItem(QByteArray data, std::function<T(QJsonObject)>dataToItem) {
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError) {
    return T();
  }
  return dataToItem(doc.object());
}

/// parseJSONItem parses a single item (assumed to be a Json Object) from a byte[]. This version
/// returns any encountered parsing errors as a QJsonParseError. Callers should check if an error
/// was encountered by performing the check: if (err.error != QJsonParseError::NoError) (entering
/// this block indicates that some error was encountered)
template <typename T>
static T parseJSONItem(QByteArray data, T (*dataToItem)(QJsonObject, QJsonParseError)) {
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError) {
    return dataToItem(QJsonObject(), err);
  }
  return dataToItem(doc.object(), err);
}

static void parseJSONItemV2(QByteArray data, std::function<void(QJsonObject, QJsonParseError)> parse) {
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError) {
    return parse(QJsonObject(), err);
  }
  parse(doc.object(), err);
}

#endif  // JSONHELPERS_H
