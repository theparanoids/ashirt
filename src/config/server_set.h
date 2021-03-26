// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef CONFIG_SERVER_H
#define CONFIG_SERVER_H

#include "appsettings.h"
#include "helpers/file_helpers.h"
#include "server_item.h"

/**
 * @brief The ServerSet class represents a standard set of functionality for any given server
 * configuration "file". This comes on the heel of ConfigV2, and as such, ServersV1 is reserved,
 * and numbering of servers implementing ServerSet begins at 2.
 */
class ServerSet {
 public:
  enum DeleteType {
    PLAIN_DELETE,
    PERMANENT_DELETE,
    RESTORE,
  };

 public:
  virtual ~ServerSet() {};

 public:
  /// writeToFile attempts to copy the in-memory servers list to the given file path.
  /// @throws an exception if writing fails
  virtual void writeToFile(QString filepath) {
    QByteArray data = toFileEncoding();
    FileHelpers::writeFile(filepath, data);
  }

  virtual void parseBytes(QByteArray content) {
    parseConfigData(content);
  }

  virtual void readFromServers(QString filepath) {
    readResult = FileHelpers::readFileNoError(filepath);
    if(readResult.success) {
      parseConfigData(readResult.data);
      setLoadedPath(filepath);
      return;
    }
    else if(!readResult.fileExists){
      makeDefaultServerset();
      setLoadedPath(filepath);
      try {
        writeToFile(filepath);
      }
      catch(std::exception &e) {} // make a best effort attempt at saving default  values
    }
    else {
      valid = false;
    }
  }

  bool isValid() {
    return valid;
  }
  FileReadResult getReadResult() {
    return readResult;
  }

 public:
  virtual ServerItem getServerByUuid(const QString& uuid) = 0;
  virtual std::vector<ServerItem> getServers(bool includeDeleted) = 0;
  virtual bool deleteServer(const QString& uuid, DeleteType deleteAction=PLAIN_DELETE) = 0;
  virtual bool addServer(ServerItem item) = 0;
  virtual bool updateServer(ServerItem item) = 0;
  virtual bool hasServer(const QString& uuid) = 0;

 protected:
  virtual void parseConfigData(const QByteArray& data) = 0;
  virtual QByteArray toFileEncoding() = 0;
  virtual QString getLoadedPath() = 0;
  virtual void setLoadedPath(QString path) = 0;
  virtual void makeDefaultServerset() {}

 private:
  bool valid = true;
  FileReadResult readResult;
};

#endif // CONFIG_SERVER_H
