// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef DATABASEERR_H
#define DATABASEERR_H

#include <QString>

#include <stdexcept>

class BadDatabaseStateError : public std::runtime_error {
 public:
  BadDatabaseStateError() : std::runtime_error("Database is in an inconsistent state") {}
};

class DBDriverUnavailableError : public std::runtime_error {
 public:
  DBDriverUnavailableError(std::string friendlyDriverName)
      : std::runtime_error(friendlyDriverName + " driver is unavailable") {}
};

class BadDBData : public std::runtime_error {
 public:
  BadDBData(QString msg) : std::runtime_error("Bad Data: " + msg.toStdString()) {}
};

#endif  // DATABASEERR_H
