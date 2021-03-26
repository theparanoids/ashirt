// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QString>
#include <QVariant>

#include "forms/evidence_filter/evidencefilter.h"
#include "models/evidence.h"
#include "helpers/constants.h"
#include "db/query_result.h"

using FieldEncoderFunc = std::function<QVariantList(unsigned int)>;
using RowDecoderFunc = std::function<void(const QSqlQuery&)>;

class DBQuery {
 private:
  QString _query;
  std::vector<QVariant> _values;

 public:
  DBQuery(QString query) : DBQuery(query, {}) {}
  DBQuery(QString query, std::vector<QVariant> values) {
    this->_query = query;
    this->_values = values;
  }
  inline QString query() { return _query; }
  inline std::vector<QVariant> values() { return _values; }
};

/**
 * @brief The DatabaseConnection class manages a connection to the database and provides an interface
 *        into common operations
 */
class DatabaseConnection {
 public:

  /**
   * @brief DatabaseConnection constructs a connection to a specific sqlite database.
   *
   * @see Constants::dbLocation() for the location to the database file
   * @throws DBDriverUnavailable if the required database driver does not exist
   * @throws QSqlError in other cases
   */
  DatabaseConnection(const QString& dbPath, QString databaseName);

  /**
   * @brief withConnection acts as a context manager for a single database connection. The goal for
   * this method is for interaction with secondary databases (e.g. during import/export), where
   * interaction is localized and infrequent.
   * @param dbPath The path to the database file
   * @param dbName The name of the database connection (can be anything, but should be unique.
   * Specifically, it should NOT be the value from Constants::databaseName())
   * @param actions A function that will execute after a connection is established. This is where
   * all db interactions should occur.
   */
  static void withConnection(const QString& dbPath, const QString &dbName,
                             const std::function<void(DatabaseConnection)> &actions);

  void connect();
  void close() noexcept;

  static DBQuery buildGetEvidenceWithFiltersQuery(const EvidenceFilters &filters);
  bool hasAppliedSystemMigration(QString systemMigrationName);
  qint64 applySystemMigration(QString systemMigrationName);

  model::Evidence getEvidenceDetails(qint64 evidenceID);
  std::vector<model::Evidence> getEvidenceWithFilters(const EvidenceFilters &filters);

  qint64 createEvidence(const QString &filepath, const QString &operationSlug,
                        const QString &serverUuid, const QString &contentType);
  qint64 createFullEvidence(const model::Evidence &evidence);
  void batchCopyFullEvidence(const std::vector<model::Evidence> &evidence);
  qint64 copyFullEvidence(const model::Evidence &evidence);
  void updateEvidencePath(const QString& newPath, qint64 evidenceID);
  void deleteEvidence(qint64 evidenceID);
  void updateEvidenceDescription(const QString &newDescription, qint64 evidenceID);
  void updateEvidenceError(const QString &errorText, qint64 evidenceID);
  void updateEvidenceSubmitted(qint64 evidenceID);

  std::vector<QString> operationSlugsForServer(const QString &operationSlug);

  void setEvidenceTags(const std::vector<model::Tag> &newTags, qint64 evidenceID);
  void batchCopyTags(const std::vector<model::Tag> &allTags);
  std::vector<model::Tag> getFullTagsForEvidenceIDs(const std::vector<qint64>& evidenceIDs);

  /// createEvidenceExportView duplicates the normal database with only a subset of evidence
  /// present, as well as related data (e.g. tags)
  ///
  /// Note that currently, this simply exports everything. This is included as a way to limit
  /// sharing in the future.
  static std::vector<model::Evidence> createEvidenceExportView(const QString& pathToExport,
                                                               const EvidenceFilters& filters,
                                                               DatabaseConnection *runningDB);
  std::vector<model::Tag> getTagsForEvidenceID(qint64 evidenceID);

  /// getDatabasePath returns the filepath associated with the loaded database
  QString getDatabasePath();

  

 public:
  const unsigned long SQLITE_MAX_VARS = 999;

 private:
  QString dbName = "";
  QString _dbPath = "";

  /**
   * @brief getSingleField retrieves a single value/cell from the provided query. If the query
   *        returns no rows, then an invalid QVariant is returned instead.
   * @param query The query to execute
   * @param args any bindings needed to execute the statement
   * @return a QVariant holding the result, or an invalid QVariant if no rows are returned
   * @throws QSqlError when a query error occurs
   */
  QVariant getSingleField(const QString& query, const std::vector<QVariant> &args);

  /**
   * @brief migrateDB checks the migration status and then performs the full migration for any
   *        lacking update.
   * @throws exceptions/FileError if a migration file cannot be found.
   */
  void migrateDB();

  QSqlDatabase getDB();

  /**
   * @brief getUnappliedMigrations Retrieves a list of all of the migrations that have not been
   *        applied to the given database.
   *        Note: All sql files must end in ".sql" to be picked up
   * @param db the database connection
   * @return A list of all of the migrations that have not yet been applied
   * @throws BadDatabaseStateError if some migrations have been applied that are not known
   * @throws QSqlError if database queries fail
   */
  static QStringList getUnappliedMigrations(const QSqlDatabase &db);

  /**
   * @brief extractMigrateUpContent Parses the given migration content and retrieves only the
   *        portion that applies to the "up" / apply logic. The "down" section is ignored. The
   *        statements are interpreted via a line with a single semicolon separating multiple
   *        statements.
   * @param allContent The (raw) contents of the sql migration script
   * @return a QStringList containing each segment for the 'up' section
   */
  static QStringList extractMigrateUpContent(const QString &allContent) noexcept;

  /**
   * @brief executeQuery Attempts to execute the given stmt with the passed args. The statement is
   *        first prepared, and arg placements can be specified with "?".
   * @param db the database connection
   * @param stmt the query to execute
   * @param args any bindings needed to execute the statement
   * @return the executed query / the effective result set
   * @throws QSqlError when a query error occurs
   */
  static QSqlQuery executeQuery(const QSqlDatabase &db, const QString &stmt,
                                const std::vector<QVariant> &args = {});

  /// executeQueryNoThrow provides a safe mechanism to execute a query on the database. (Safe in the
  /// sense that no exception is thrown). It is incumbent on the caller to inspect the
  /// QueryResult.sucess/QueryResult.err fields to determine the actual result.
  static QueryResult executeQueryNoThrow(const QSqlDatabase &db, const QString &stmt,
                                         const std::vector<QVariant> &args = {}) noexcept;
  
  /**
   * @brief doInsert is a version of executeQuery that returns the last inserted id, rather than the
   *        underlying query/response.
   * @param db the database connection
   * @param stmt the query to execute
   * @param args any bindings needed to execute the statement
   * @return the last inserted id if successful
   * @throws QSqlError when a query error occurs
   */
  static qint64 doInsert(const QSqlDatabase &db, const QString &stmt, const std::vector<QVariant> &args);

  /**
   * @brief batchInsert batches multiple inserts over as few requests as possible.
   * @param baseQuery The insert string, up to " VALUES "
   * @param varsPerRow the number of items per row
   * @param numRows the number of rows you wish to insert
   * @param encodeValues A function that, given a row index, will return a QVariantList with each column's data for that row
   * @param rowInsertTemplate An optional string that can be used to define each row's values. Defaults to (?, ..., ?)
   */
  void batchInsert(const QString& baseQuery, unsigned int varsPerRow, unsigned int numRows,
                   const FieldEncoderFunc& encodeValues, QString rowInsertTemplate="");

  /**
   * @brief batchQuery batches a query with many variables into as few queries as possible.
   * Note: All variables need to be in the same location in the query.
   * @param baseQuery A QString template query, with the variables denoted with "%1"
   * @param varsPerRow The number of variables per requested (expected: 1)
   * @param numRows the number of variables to placve
   * @param encodeValues A function that, given an index, returns a QVariantList for each variable group
   * @param decodeRows A function that can be used to retrieve the rows from the result set
   * @param variableTemplate An optional string that can be used to define how variables are handled. Defaults to ?,...,?
   */
  void batchQuery(const QString &baseQuery, unsigned int varsPerRow, unsigned int numRows,
                  const FieldEncoderFunc &encodeValues, const RowDecoderFunc& decodeRows,
                  QString variableTemplate = "");
};

#endif  // DATABASECONNECTION_H
