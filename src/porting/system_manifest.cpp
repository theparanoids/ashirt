#include "system_manifest.h"

using namespace porting;

void SystemManifest::applyManifest(SystemManifestImportOptions options, DatabaseConnection* systemDb) {
  bool shouldMigrateConfig = options.importConfig && !configPath.isEmpty();
  bool shouldMigrateDb = options.importDb == SystemManifestImportOptions::Merge && !dbPath.isEmpty();
  bool shouldMigrateServers = options.importServers && !serversPath.isEmpty();

  if (shouldMigrateConfig) {
    emit onStatusUpdate("Importing Settings");
    migrateConfig();
  }

  if (shouldMigrateServers) {
    emit onStatusUpdate("Importing Servers");
    migrateServers();
  }


  if (shouldMigrateDb) {
    migrateDb(systemDb);
  }
  emit onComplete();
}

void SystemManifest::migrateConfig() {
  auto data = FileHelpers::readFile(pathToFile(configPath));

  auto config = AppConfig::getInstance().parseConfig(data);
  // ignoring EvidencePath, as this can be dangerous to overwrite
  AppConfig::getInstance().setIfEmpty(Config::Fields::CaptureCodeblockShortcut, config->captureCodeblockShortcut());
  AppConfig::getInstance().setIfEmpty(Config::Fields::CaptureScreenAreaCmd, config->captureScreenAreaCmd());
  AppConfig::getInstance().setIfEmpty(Config::Fields::CaptureScreenAreaShortcut, config->captureScreenAreaShortcut());
  AppConfig::getInstance().setIfEmpty(Config::Fields::CaptureScreenWindowCmd, config->captureScreenWindowCmd());
  AppConfig::getInstance().setIfEmpty(Config::Fields::CaptureScreenWindowShortcut, config->captureScreenWindowShortcut());

  // allow migrations from old style config (v1, which included server info) to the new style
  // (servers hosted via AppServers)
  if(config->version() == 1) {
    auto cfg = (ConfigV1*)config;
    auto legacyServer = AppServers::getInstance().getServerByUuid(Constants::legacyServerUuid());
    if( legacyServer.getServerUuid().isEmpty() ) {
      AppServers::getInstance().addServer(
        ServerItem(Constants::defaultServerName(), cfg->accessKey(), cfg->secretKey(), cfg->apiURL())
      );
      AppServers::getInstance().writeServers();
    }
  }

  AppConfig::getInstance().writeConfig(); // save updated config
}

void SystemManifest::migrateServers() {
  auto data = FileHelpers::readFile(pathToFile(serversPath));
  auto serverData = AppServers::getInstance().parseServers(data);

  // merge only unknown servers
  for (auto importServer : serverData->getServers(false)) {
    auto foundServer = AppServers::getInstance().getServerByUuid(importServer.getServerUuid());
    if (foundServer.getServerUuid().isEmpty()) {
      AppServers::getInstance().addServer(importServer);
    }
  }
  AppServers::getInstance().writeServers();
}

void SystemManifest::migrateDb(DatabaseConnection* systemDb) {
  emit onStatusUpdate("Reading Exported Evidence");
  auto evidenceManifest = EvidenceManifest::deserialize(pathToFile(evidenceManifestPath));
  onReady(evidenceManifest.entries.size());
  DatabaseConnection::withConnection(
      pathToFile(dbPath), "importDb", [this, evidenceManifest, systemDb](DatabaseConnection importDb) {
        emit onStatusUpdate("Importing evidence");
        for (size_t entryIndex = 0; entryIndex < evidenceManifest.entries.size(); entryIndex++) {
          emit onFileProcessed(entryIndex); // this only makes sense on the 2nd+ iteration, but this works since indexes start at 0
          auto item = evidenceManifest.entries.at(entryIndex);
          auto importRecord = importDb.getEvidenceDetails(item.evidenceID);
          if (importRecord.id == 0) {
            continue; // in the odd situation that evidence doesn't match up, just skip it
          }
          QString newEvidencePath = AppConfig::getInstance().evidenceRepo() + "/" +
                                    importRecord.operationSlug + "/" +
                                    contentSensitiveFilename(importRecord.contentType);
          QString newEviPathLastFour = newEvidencePath.right(4);

          auto fullFileExportPath = pathToManifest + "/" + item.exportPath;
          auto copyResult = FileHelpers::copyFile(fullFileExportPath, newEvidencePath, true);

          if (!copyResult.success) {
            emit onCopyFileError(fullFileExportPath, newEvidencePath,
                                 FileError::mkError(copyResult.file->errorString(), newEvidencePath, copyResult.file->error()));
          }

          importRecord.path = newEvidencePath;
          qint64 evidenceID = systemDb->createFullEvidence(importRecord);
          systemDb->setEvidenceTags(importRecord.tags, evidenceID);
       }
       emit onFileProcessed(evidenceManifest.entries.size()); // update the full set now that this is complete
  });
}

QString SystemManifest::pathToFile(const QString& filename) {
  return pathToManifest + "/" + filename;
}

QString SystemManifest::contentSensitiveExtension(const QString& contentType) {
  if (contentType == Codeblock::contentType()) {
    return Codeblock::extension();
  }
  else if(contentType == Screenshot::contentType()) {
    return Screenshot::extension();
  }
  return ".bin";
}

QString SystemManifest::contentSensitiveFilename(const QString& contentType) {
  if (contentType == Codeblock::contentType()) {
    return Codeblock::mkName();
  }
  else if(contentType == Screenshot::contentType()) {
    return Screenshot::mkName();
  }
  return FileHelpers::randomFilename("ashirt_unknown_type_XXXXXX.bin");
}

SystemManifest* SystemManifest::readManifest(const QString& pathToExportFile) {
  auto content = FileHelpers::readFile(pathToExportFile);
  auto manifest = parseJSONItem<SystemManifest*>(content, &SystemManifest::deserialize);
  manifest->pathToManifest = FileHelpers::getDirname(pathToExportFile);

  return manifest;
}

void SystemManifest::exportManifest(DatabaseConnection* db, const QString& outputDirPath,
                                    const SystemManifestExportOptions& options) {
  if (!options.includesAnything()) {
    return;
  }

  bool success = FileHelpers::mkdirs(outputDirPath);
  if (!success) {
    return;
  }

  os = QSysInfo::kernelType(); // may need to check possible answers, or maybe just compare to new system value?

  QString basePath = QDir(outputDirPath).path();

  if (options.exportConfig) {
    emit onStatusUpdate("Exporting settings");
    configPath = "config.json";
    AppConfig::getInstance().writeConfig(basePath + "/" + configPath);
  }

  if (options.exportServers) {
    emit onStatusUpdate("Exporting servers");
    serversPath = "servers.json";
    AppServers::getInstance().writeServers(basePath + "/" + serversPath);
  }

  if (options.exportDb) {
    emit onStatusUpdate("Exporting Evidence");
    dbPath = "db.sqlite";
    evidenceManifestPath = "evidence.json";

    auto allEvidence = DatabaseConnection::createEvidenceExportView(basePath + "/" + dbPath, EvidenceFilters(), db);
    emit onReady(allEvidence.size());
    porting::EvidenceManifest evidenceManifest = copyEvidence(basePath, allEvidence);

    // write evidence manifest
    FileHelpers::writeFile(basePath + "/" + evidenceManifestPath,
                           QJsonDocument(EvidenceManifest::serialize(evidenceManifest)).toJson());
  }

  QString exportPath = basePath + "/system.json";
  this->pathToManifest = exportPath;
  FileHelpers::writeFile(exportPath, QJsonDocument(serialize(*this)).toJson());
  emit onComplete();
}

porting::EvidenceManifest SystemManifest::copyEvidence(const QString& baseExportPath,
                                                       std::vector<model::Evidence> allEvidence) {
  QString relativeEvidenceDir = "evidence";
  FileHelpers::mkdirs(baseExportPath + "/" + relativeEvidenceDir);

  porting::EvidenceManifest evidenceManifest;
  for (size_t evidenceIndex = 0; evidenceIndex < allEvidence.size(); evidenceIndex++) {
    auto evi = allEvidence.at(evidenceIndex);
    QString randPart = "??????????";
    auto filenameTemplate = QString("ashirt_evidence_%1.%2")
                                .arg(randPart, contentSensitiveExtension(evi.contentType));
    QString newName = FileHelpers::randomFilename(filenameTemplate, randPart);
    auto item = porting::EvidenceItem(evi.id, relativeEvidenceDir + "/" + newName);
    auto dstPath = baseExportPath + "/" + item.exportPath;
    auto copyResult = FileHelpers::copyFile(evi.path, dstPath);

    if (!copyResult.success) {
      emit onCopyFileError(evi.path, dstPath,
                           FileError::mkError(copyResult.file->errorString(), dstPath, copyResult.file->error()));
    }
    else {
      evidenceManifest.entries.push_back(item);
    }
    emit onFileProcessed(evidenceIndex + 1);
  }
  return evidenceManifest;
}

QJsonObject SystemManifest::serialize(const SystemManifest& src) {
  QJsonObject o;
  o.insert("operatingSystem", src.os);
  o.insert("databasePath", src.dbPath);
  o.insert("configPath", src.configPath);
  o.insert("serversPath", src.serversPath);
  o.insert("evidenceManifestPath", src.evidenceManifestPath);
  return o;
}

SystemManifest* SystemManifest::deserialize(const QJsonObject& o) {
  auto manifest = new SystemManifest;
  manifest->os = o.value("operatingSystem").toString();
  manifest->dbPath = o.value("databasePath").toString();
  manifest->configPath = o.value("configPath").toString();
  manifest->serversPath = o.value("serversPath").toString();
  manifest->evidenceManifestPath = o.value("evidenceManifestPath").toString();
  return manifest;
}
