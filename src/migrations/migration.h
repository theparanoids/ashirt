// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef MIGRATION_H
#define MIGRATION_H

#include <QString>

#include "db/databaseconnection.h"

class Migration {

 public:
  virtual ~Migration() {};

 public:
  static bool applyMigrations(DatabaseConnection* conn);

 public:
  virtual QString migrationName() = 0;

 protected:
  virtual bool doMigration(DatabaseConnection* conn) = 0;
};

#endif // MIGRATION_H
