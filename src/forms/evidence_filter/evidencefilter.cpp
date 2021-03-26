// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "evidencefilter.h"

#include "appservers.h"

QString EvidenceFilters::standardizeFilterKey(QString key) {
  if (FILTER_KEYS_ERROR.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_ERROR;
  }
  if (FILTER_KEYS_SUBMITTED.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_SUBMITTED;
  }
  if (FILTER_KEYS_TO.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_TO;
  }
  if (FILTER_KEYS_FROM.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_FROM;
  }
  if (FILTER_KEYS_ON.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_ON;
  }
  if (FILTER_KEYS_OPERATION.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_OPERATION;
  }
  if (FILTER_KEYS_CONTENT_TYPE.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_CONTENT_TYPE;
  }
  if (FILTER_KEYS_SERVER_UUID.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_SERVER_UUID;
  }
  if (FILTER_KEYS_SERVER_NAME.contains(key, Qt::CaseInsensitive)) {
    return FILTER_KEY_SERVER_NAME;
  }
  return key;
}

QString EvidenceFilters::toString() const {
  // helper functions
  static auto toCommonDate = [](QDate d) -> QString {
    auto today = QDateTime::currentDateTime().date();
    if (d == today) {
      return "Today";
    }
    if (d == today.addDays(-1)) {
      return "Yesterday";
    }
    return d.toString("yyyy-MM-dd");
  };
  static auto triToText = [](Tri t) -> QString { return t == Yes ? "yes" : "no"; };

  QString fieldTemplate = " %1: %2"; //intentional space to provide gaps between fields
  auto quote = [](QString val){ return QString(R"("%1")").arg(val); };

  QString rtn = "";
  auto append = [&rtn, fieldTemplate](QString key, QString value){
    rtn.append(fieldTemplate.arg(key, value));
  };

  if (!operationSlug.isEmpty()) {
    append(FILTER_KEY_OPERATION, operationSlug);
  }
  if (!contentType.isEmpty()) {
    append(FILTER_KEY_CONTENT_TYPE, contentType);
  }
  if (hasError != Any) {
    append(FILTER_KEY_ERROR, triToText(hasError));
  }
  if (startDate.isValid() || endDate.isValid()) {
    if (startDate == endDate) {
      append(FILTER_KEY_ON, toCommonDate(startDate));
    }
    else if (startDate.isValid()) {
      append(FILTER_KEY_FROM, toCommonDate(startDate));
    }
    else { // endDate must be valid
      append(FILTER_KEY_TO, toCommonDate(endDate));
    }
  }
  if (submitted != Any) {
    append(FILTER_KEY_SUBMITTED, triToText(submitted));
  }
  if (!serverUuid.isEmpty()) {
    append(FILTER_KEY_SERVER_NAME, quote(getServerName()));
  }

  return rtn.trimmed();
}

EvidenceFilters EvidenceFilters::parseFilter(const QString& text) {
  EvidenceFilters filter;
  if (text.trimmed().isEmpty()) {
    return filter;
  }

  auto tokenizedFilter = tokenizeFilterText(text);

  for (const auto& item : tokenizedFilter) {
    QString key = EvidenceFilters::standardizeFilterKey(item.first.toLower().trimmed());
    QString value = item.second.trimmed();

    if (key == FILTER_KEY_ERROR) {
      auto val = value.toLower();
      filter.hasError = parseTriFilterValue(val);
    }
    else if (key == FILTER_KEY_SUBMITTED) {
      auto val = value.toLower();
      filter.submitted = parseTriFilterValue(val);
    }
    else if (key == FILTER_KEY_OPERATION) {
      filter.operationSlug = value;
    }
    else if (key == FILTER_KEY_SERVER_UUID || key == FILTER_KEY_SERVER_NAME) {
      // these speak to the same field -- prefer the value in serverUuid
      if (key == FILTER_KEY_SERVER_UUID) {
        filter.serverUuid = value;
      }
      else {
        filter.setServerByName(value);
      }
    }
    else if (key == FILTER_KEY_TO) {
      filter.endDate = parseDateString(value);
    }
    else if (key == FILTER_KEY_FROM) {
      filter.startDate = parseDateString(value);
    }
    else if (key == FILTER_KEY_ON) {
      auto formattedValue = parseDateString(value);
      filter.startDate = formattedValue;
      filter.endDate = formattedValue;
    }
    else if (key == FILTER_KEY_CONTENT_TYPE) {
      filter.contentType = value;
    }
  }

  return filter;
}

Tri EvidenceFilters::parseTriFilterValue(const QString& text, bool strict) {
  auto val = text.toLower().trimmed();
  if (val.startsWith("t") || val.startsWith("y")) {
    return Tri::Yes;
  }
  if (strict) {
    return (val.startsWith("f") || val.startsWith("n")) ? Tri::No : Tri::Any;
  }
  return Tri::No;
}

std::vector<std::pair<QString, QString>> EvidenceFilters::tokenizeFilterText(const QString& text) {
  bool inQuote = false;
  int startWordIndex = 0;
  std::vector<std::pair<QString, QString>> rtn;

  bool hasValue = false;
  int sepLocation = -1;

  // maybeAddWord performs some validation and standardization on the given area. If the validation
  // succeeds, then the "word" is added. Otherwise, it's dropped.
  auto maybeAddWord = [&rtn, &hasValue, text](int startLocation, int sepLocation, int endLocation) {
    hasValue = false;
    if (sepLocation == -1) {
      return;
    }

    // key and value trim before replacing to allow for leading and/or ending spaces
    QString key = text.mid(startLocation, sepLocation - startLocation).trimmed().replace("\"", "");
    QString value = text.mid(sepLocation+1, endLocation - sepLocation).trimmed().replace("\"", "");
    if (key == "" || value == "") {
      return;
    }
    QString adjustedPhrase = key + ":" + value;

    rtn.push_back(std::pair<QString, QString>(key, value));
  };

  for(int i = 0; i < text.size(); i++) {
    QChar ch = text.at(i);
    bool addWord = false;

    if(sepLocation > -1 && !ch.isSpace()) {
      hasValue = true;
    }

    if(ch == '\"') {
      inQuote = !inQuote;
      if(inQuote == false) {
        addWord = true;
      }
    }
    else if(ch == ':' && !inQuote && sepLocation == -1) {
      sepLocation = i;
    }
    else if(ch.isSpace() && !inQuote) {
      addWord = true;
    }

    if(addWord && hasValue) {
      maybeAddWord(startWordIndex, sepLocation, i);
      sepLocation = -1;
      startWordIndex = i+1;
    }
  }
  maybeAddWord(startWordIndex, sepLocation, text.length()-1);

  return rtn;
}

QDate EvidenceFilters::parseDateString(QString text) {
  static QString DATE_FORMAT = "yyyy-MM-dd";

  text = text.toLower();
  if (text == "today") {
    text = QDateTime::currentDateTime().toString(DATE_FORMAT);
  }
  else if (text == "yesterday") {
    text = QDateTime::currentDateTime().addDays(-1).toString(DATE_FORMAT);
  }
  // other thoughts: this week (hard -- spans years), this month, this year, last ${period}

  return QDate::fromString(text, DATE_FORMAT);
}

Tri EvidenceFilters::parseTri(const QString& text) {
  if (text == "Yes") {
    return Yes;
  }
  if (text == "No") {
    return No;
  }
  return Any;
}

QString EvidenceFilters::triToString(const Tri& tri) {
  switch (tri) {
    case Yes:
      return "Yes";
    case No:
      return "No";
    default:
      return "Any";
  }
}

void EvidenceFilters::setServer(QString serverUuid) {
  this->serverUuid = serverUuid;
}

QString EvidenceFilters::getServerUuid() {
  return serverUuid;
}
QString EvidenceFilters::getServerUuid() const {
  return serverUuid;
}

QString EvidenceFilters::getServerName() {
  return serverUuid == "" ? "" : AppServers::getInstance().serverName(serverUuid);
}

QString EvidenceFilters::getServerName() const {
  return serverUuid == "" ? "" : AppServers::getInstance().serverName(serverUuid);
}


void EvidenceFilters::setServerByName(QString serverName) {
  auto list = AppServers::getInstance().getServers(true);
  for(auto server : list) {
    if (server.serverName == serverName) {
      setServer(server.getServerUuid());
    }
  }
}


///////////////////////////////////////////////////////////////////////////
// once unit tests are available, the should serve as a good starting point
///////////////////////////////////////////////////////////////////////////


//void runTests() {
//  auto matches = [](std::vector<std::pair<QString, QString>> a, std::vector<std::pair<QString, QString>> b, QString label="") {
//    if (label != "") {
//      auto copy = QString(label);
//      copy.resize(30, ' ');
//      std::cout << "[" << copy.toStdString() << "]     ";
//    }

//    if (a.size() != b.size()) {
//      std::cout << "No match: size difference" << std::endl;
//      return false;
//    }

//    for(size_t i = 0; i < a.size(); i++) {
//      auto expected = a[i];
//      auto actual = b[i];
//      if (expected.first != actual.first || expected.second != actual.second) {
//        std::cout << "No match: [" << expected.first.toStdString() << "]!=[" << actual.first.toStdString()
//                  << "] || [" << expected.second.toStdString() << "]!=[" << actual.second.toStdString()
//                  << "]\n";
//        return false;
//      }
//    }
//    std::cout << "okay!" << std::endl;

//    return true;
//  };
//  auto mkpair = [](QString a, QString b){ return std::pair<QString,QString>(a, b); };

//  matches(tokenizeFilterText("a:b"), {mkpair("a", "b")}, "single-plain");
//  matches(tokenizeFilterText("a:b c:d"), {mkpair("a", "b"), mkpair("c", "d")}, "double-plain");
//  matches(tokenizeFilterText("a:b  c:d"), {mkpair("a", "b"), mkpair("c", "d")}, "double-extra-space");
//  matches(tokenizeFilterText("a:b c:d "), {mkpair("a", "b"), mkpair("c", "d")}, "double-trailing-plain");
//  matches(tokenizeFilterText("a :b c :d"), {mkpair("a", "b"), mkpair("c", "d")}, "left-spaced-sep");
//  matches(tokenizeFilterText("a: b c: d"), {mkpair("a", "b"), mkpair("c", "d")}, "right-spaced-sep");
//  matches(tokenizeFilterText("a : b c : d"), {mkpair("a", "b"), mkpair("c", "d")}, "double-spaced-sep");
//  matches(tokenizeFilterText("name:\"joel smith\" age:38"), {mkpair("name", "joel smith"), mkpair("age", "38")}, "plain-quote");
//  matches(tokenizeFilterText("name:\"joel smith\"age:38"), {mkpair("name", "joel smith"), mkpair("age", "38")}, "no-gap-quote-val");
//  matches(tokenizeFilterText("name:\"joel : smith\" age:38"), {mkpair("name", "joel : smith"), mkpair("age", "38")}, "sep-in-quote-val");
//  matches(tokenizeFilterText("\"name\":\"joel smith\" age:38"), {mkpair("name", "joel smith"), mkpair("age", "38")}, "quoted-key");

//  matches(tokenizeFilterText("a:b c"), {mkpair("a", "b")}, "malformed-extra-endjunk");
//  matches(tokenizeFilterText("! a:b"), {mkpair("! a", "b")}, "malformed-extra-startjunk");

//  matches(tokenizeFilterText("op:HPCoS server:abcdefgh-0123-0123-0123-0123456789AB"), {mkpair("op", "HPCoS"), mkpair("server", "abcdefgh-0123-0123-0123-0123456789AB")}, "real-life");
//}

