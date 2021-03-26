// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "migration.h"

#include <vector>

#include "migrations/multi_server_migration.h"

bool Migration::applyMigrations(DatabaseConnection* conn) {
  std::vector<Migration*> toApply = {
      new MultiServerMigration(),
      // new migrations go below, as these are executed in sequential order
  };

  for (Migration* task : toApply) {
    if(!conn->hasAppliedSystemMigration(task->migrationName())) {
      std::cout << "Applying system migration: " << task->migrationName().toStdString() << std::endl;
      bool success = task->doMigration(conn);
      if(success) {
        conn->applySystemMigration(task->migrationName());
      }
      else {
        return false;
      }
    }
  }
  std::cout << "All system migrations applied" << std::endl;

  for (auto completed : toApply) {
    delete completed;
  }

  return true;
}
