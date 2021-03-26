// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef EVIDENCEFILTER_H
#define EVIDENCEFILTER_H

#include <QDate>
#include <QString>
#include <utility>
#include <vector>
#include <QStringList>

enum Tri { Any, Yes, No };

// These represent the standard key for a filter
const QString FILTER_KEY_ERROR = "err";
const QString FILTER_KEY_SUBMITTED = "submitted";
const QString FILTER_KEY_TO = "to";
const QString FILTER_KEY_FROM = "from";
const QString FILTER_KEY_ON = "on";
const QString FILTER_KEY_OPERATION = "op";
const QString FILTER_KEY_CONTENT_TYPE = "type";
const QString FILTER_KEY_SERVER_UUID = "serverUuid";
const QString FILTER_KEY_SERVER_NAME = "server";


// These represent aliases for standard key for a filter
const QStringList FILTER_KEYS_ERROR = {FILTER_KEY_ERROR, "error", "failed", "fail"};
const QStringList FILTER_KEYS_SUBMITTED = {FILTER_KEY_SUBMITTED};
const QStringList FILTER_KEYS_TO = {FILTER_KEY_TO, "before", "til", "until"};
const QStringList FILTER_KEYS_FROM = {FILTER_KEY_FROM, "after"};
const QStringList FILTER_KEYS_ON = {FILTER_KEY_ON};
const QStringList FILTER_KEYS_OPERATION = {FILTER_KEY_OPERATION, "operation"};
const QStringList FILTER_KEYS_CONTENT_TYPE = {FILTER_KEY_CONTENT_TYPE, "contentType", "content_type"};
const QStringList FILTER_KEYS_SERVER_UUID = {FILTER_KEY_SERVER_UUID, "serverUUID", "server_uuid"};
const QStringList FILTER_KEYS_SERVER_NAME = {FILTER_KEY_SERVER_NAME, "serverName", "server_name", "backend"};

class EvidenceFilters {
 public:
  EvidenceFilters() = default;

 public:
  /// standardizeFilterKey takes the given key (hopefully defined in one of the QStringLists above)
  /// and finds the "standard" representation for that key. e.g. this will convert "operation" to "op"
  static QString standardizeFilterKey(QString key);

  /// parseFilter will attempt to convert the given string into an EvidenceFilter object.
  /// @see tokenizeFilterText
  static EvidenceFilters parseFilter(const QString &text);

  /// parseTri returns Tri::Yes if the given text is exactly "Yes", Tri::No if the text is exactly "No"
  /// otherwise Tri::Any.
  /// This is the inverse of triToString
  static Tri parseTri(const QString &text);

  /// triToString returns "Yes" for Tri::Yes, "No" for Tri::No, otherwise "Any"
  /// This is the inverse to parseTri
  static QString triToString(const Tri &tri);

 public:
  /// toString converts the EvidenceFilter into a standard string representation
  /// This can then be passed to itself to re-generate a EvidenceFilter. In essence, this is a
  /// human readable serialization format (when paired with parseFilter)
  /// @see parseFilter
  QString toString() const;

  void setServer(QString serverUuid);
  QString getServerUuid() const;
  QString getServerUuid();
  QString getServerName();
  QString getServerName() const;

  void setServerByName(QString serverName);


 private:
  /**
   * @brief tokenizeFilterText splits up a string into a list of key-value pairs, in the order they were
   * encountered. The mechanics are complicated.
   *
   * Text should be of the general form: [keyword]:[value] (repeated for each pair needed).
   * Keywords should contain no colons or spaces (unless in quotes, but this is not preferred).
   * Underscores, dashes, etc, are allowed.
   * Values can be multiword, but do need to be enclosed by quotes if they use spaces.
   * Colons separate the key from the value. Colons may be used in the value.
   *
   * Input vs Expected Output
   * "tag : value" => {("tag", "value")}
   * "tag1:value1 tag2:value2" => {("tag1", "value1"), ("tag2", "value2")}
   * 'name: "George Washington" born:"February 22, 1732" aged:67' =>
   *    {("name", "George Washington")("born", "February 22, 1732"), ("aged", "67")}
   *
   * @param text the given text to be tokenized
   * @return a list of key/value words, in an encountered-ordered list
   */
  static std::vector<std::pair<QString, QString>> tokenizeFilterText(const QString &text);
  static QDate parseDateString(QString text);

  /// parseTriFilterValue returns a Tri object given a string. If the given string is "t" or "y"
  /// then Tri::Yes will be returned. Otherwise, in non-strict mode, Tri::No will be returned.
  /// In strict mode, Tri::No will be returned only if it starts with "f" or "n", otherwise Tri::Any
  /// is returned.
  static Tri parseTriFilterValue(const QString &text, bool strict = false);

 public:
  QString operationSlug = "";
  QString contentType = "";
  Tri hasError = Any;
  Tri submitted = Any;
  QDate startDate = QDate();
  QDate endDate = QDate();

 protected:
  QString serverUuid = "";
};

#endif  // EVIDENCEFILTER_H
