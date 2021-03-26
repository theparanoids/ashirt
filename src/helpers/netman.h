// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef NETMAN_H
#define NETMAN_H

#include <QMessageAuthenticationCode>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "appconfig.h"
#include "appservers.h"
#include "request_builder.h"
#include "dtos/operation.h"
#include "dtos/tag.h"
#include "dtos/github_release.h"
#include "helpers/file_helpers.h"
#include "helpers/multipartparser.h"
#include "helpers/stopreply.h"
#include "models/evidence.h"

class NetMan : public QObject {
  Q_OBJECT

 public:
  static NetMan &getInstance() {
    static NetMan instance;
    return instance;
  }
  NetMan(NetMan const &) = delete;
  void operator=(NetMan const &) = delete;
  // type alias std::vector<dto::Operation> to provide shorter lines
  using OperationVector = std::vector<dto::Operation>;

 signals:
  void operationListUpdated(bool success, OperationVector  operations = OperationVector());
  void releasesChecked(bool success, std::vector<dto::GithubRelease> releases = std::vector<dto::GithubRelease>());
  void testConnectionComplete(bool connected, int statusCode);

 private:
  QNetworkAccessManager *nam;

  NetMan() { nam = new QNetworkAccessManager; }
  ~NetMan() {
    delete nam;
    stopReply(&allOpsReply);
    stopReply(&githubReleaseReply);
  }

  /// ashirtGet generates a basic GET request to the ashirt API server. No authentication is
  /// provided (use addASHIRTAuth to do this)
  /// Allows for an optional altHost parameter, in order to check for ashirt servers.
  /// Normal usage should provide no value for this parameter.
  RequestBuilder* ashirtGet(QString endpoint, const QString & altHost="") {
    QString base = (altHost == "") ? AppServers::getInstance().hostPath() : altHost;
    return RequestBuilder::newGet()
        ->setHost(base)
        ->setEndpoint(endpoint);
  }

  /// ashirtJSONPost generates a basic POST request with content type application/json. No
  /// authentication is provided (use addASHIRTAuth to do this)
  RequestBuilder* ashirtJSONPost(QString endpoint, QByteArray body) {
    return RequestBuilder::newJSONPost()
        ->setHost(AppServers::getInstance().hostPath())
        ->setEndpoint(endpoint)
        ->setBody(body);
  }

  /// ashirtFormPost generates a basic POST request with content type multipart/form-data.
  /// No authentication is provided (use addASHIRTAuth to do this)
  RequestBuilder* ashirtFormPost(QString endpoint, QByteArray body, QString boundry) {
    return RequestBuilder::newFormPost(boundry)
        ->setHost(AppServers::getInstance().hostPath())
        ->setEndpoint(endpoint)
        ->setBody(body);
  }

  /// addASHIRTAuth takes the provided RequestBuilder and adds on Authorization and Date headers
  /// in order to properly authenticate with ASHIRT servers. Note that this should not be used for
  /// non-ashirt requests
  void addASHIRTAuth(RequestBuilder* reqBuilder, const QString& altApiKey = "",
                     const QString& altSecretKey = "") {
    auto now = QDateTime::currentDateTimeUtc().toString("ddd, dd MMM yyyy hh:mm:ss 'GMT'");
    reqBuilder->addRawHeader("Date", now);

    // load default key if not present
    QString apiKeyCopy = QString(altApiKey);
    if (apiKeyCopy.isEmpty()) {
      apiKeyCopy = AppServers::getInstance().accessKey();
    }

    auto code = generateHash(RequestMethodToString(reqBuilder->getMethod()),
                             reqBuilder->getEndpoint(), now, reqBuilder->getBody(), altSecretKey);

    auto authValue = apiKeyCopy + ":" + code;
    reqBuilder->addRawHeader("Authorization", authValue);
  }


  /// generateHash provides a cryptographic hash for ASHIRT api server communication
  QString generateHash(QString method, QString path, QString date, QByteArray body = NO_BODY,
                       const QString &secretKey = "") {
    auto hashedBody = QCryptographicHash::hash(body, QCryptographicHash::Sha256);
    std::string msg = (method + "\n" + path + "\n" + date + "\n").toStdString();
    msg += hashedBody.toStdString();

    QString secretKeyCopy = QString(secretKey);
    if (secretKeyCopy.isEmpty()) {
      secretKeyCopy = AppServers::getInstance().secretKey();
    }

    QMessageAuthenticationCode code(QCryptographicHash::Sha256);
    QByteArray key = QByteArray::fromBase64(FileHelpers::qstringToByteArray(secretKeyCopy));

    code.setKey(key);
    code.addData(FileHelpers::stdStringToByteArray(msg));
    return code.result().toBase64();
  }

  /// onGetOpsComplete is called when the network request associated with the method refreshOperationsList
  /// completes. This will emit an operationListUpdated signal.
  void onGetOpsComplete() {
    bool success = false;
    OperationVector ops = parseOpsResponse(allOpsReply, success);
    emit operationListUpdated(success, ops);
    tidyReply(&allOpsReply);
  }

  /// onGithubReleasesComplete is called when the network request associated with the method checkForNewRelease
  /// completes. This will emit a releasesChecked signal
  void onGithubReleasesComplete() {
    bool isValid;
    auto data = extractResponse(githubReleaseReply, isValid);
    if (isValid) {
      auto releases = dto::GithubRelease::parseDataAsList(data);
      emit releasesChecked(true, releases);
    }
    else {
      emit releasesChecked(false);
    }
    tidyReply(&githubReleaseReply);
  }

 public:

  OperationVector parseOpsResponse(QNetworkReply* reply, bool &success, bool doSort=true) {
    bool isValid;
    auto data = extractResponse(reply, isValid);
    if (isValid) {
      OperationVector ops = dto::Operation::parseDataAsList(data);
      if (doSort) {
        std::sort(ops.begin(), ops.end(),
                  [](dto::Operation i, dto::Operation j) { return i.name < j.name; });
      }

      success = true;
      return ops;
    }
    else {
      success = false;
      return {};
    }
  }

  /// uploadAsset takes the given Evidence model, encodes it (and the file), and uploads this
  /// to the configured ASHIRT API server. Returns a QNetworkReply to track the request
  /// Note: does not specify the occurred_at field, so occurred_at will reflect the time of upload,
  /// rather than the time of capture.
  QNetworkReply *uploadAsset(model::Evidence evidence) {
    MultipartParser parser;
    parser.AddParameter("notes", evidence.description.toStdString());
    parser.AddParameter("contentType", evidence.contentType.toStdString());

    // TODO: convert this time below into a proper unix timestamp (mSecSinceEpoch and secsSinceEpoch
    // produce invalid times)
    // parser.AddParameter("occurred_at", std::to_string(evidence.recordedDate);

    QStringList list;
    for (auto tag : evidence.tags) {
      list << QString::number(tag.serverTagId);
    }
    parser.AddParameter("tagIds", ("[" + list.join(",") + "]").toStdString());

    parser.AddFile("file", evidence.path.toStdString());

    auto body = FileHelpers::stdStringToByteArray(parser.GenBodyContent());

    auto builder = ashirtFormPost("/api/operations/" + evidence.operationSlug + "/evidence", body, parser.boundary().c_str());
    addASHIRTAuth(builder);
    return builder->execute(nam);
  }

  /// testConnection provides a mechanism to validate a given host, apikey and secret key, to test
  /// a connection to the ASHIRT API server
  QNetworkReply *testConnection(QString host, QString apiKey, QString secretKey) {
    auto builder = ashirtGet("/api/checkconnection", host);
    addASHIRTAuth(builder, apiKey, secretKey);
    return builder->execute(nam);
  }

  /// getAllOperations retrieves all (user-visble) operations from the configured ASHIRT API server.
  /// Note: normally you should opt to use refreshOperationsList and retrieve the results by listening
  /// for the operationListUpdated signal.
  QNetworkReply *getAllOperations(const QString& host="", const QString& apiKey="", const QString& secretKey="") {
    auto builder = ashirtGet("/api/operations", host);
    addASHIRTAuth(builder, apiKey, secretKey);
    return builder->execute(nam);
  }

  /// getGithubReleases retrieves the recent releases from github for the provided owner and repo.
  /// Note that normally you should call checkForNewRelease
  QNetworkReply *getGithubReleases(QString owner, QString repo) {
    return RequestBuilder::newGet()
        ->setHost("https://api.github.com")
        ->setEndpoint("/repos/" + owner + "/" + repo + "/releases")
        ->execute(nam);
  }

  /// refreshOperationsList retrieves the operations currently visible to the user. Results should be
  /// retrieved by listening for the operationListUpdated signal
  void refreshOperationsList() {
    if (allOpsReply == nullptr) {
      allOpsReply = getAllOperations();
      connect(allOpsReply, &QNetworkReply::finished, this, &NetMan::onGetOpsComplete);
    }
  }

  /// getOperationTags retrieves the tags for specified operation from the ASHIRT API server
  QNetworkReply *getOperationTags(QString operationSlug) {
    auto builder = ashirtGet("/api/operations/" + operationSlug + "/tags");
    addASHIRTAuth(builder);
    return builder->execute(nam);
  }

  /// createTag attempts to create a new tag for specified operation from the ASHIRT API server.
  QNetworkReply *createTag(dto::Tag tag, QString operationSlug) {
    auto builder = ashirtJSONPost("/api/operations/" + operationSlug + "/tags", dto::Tag::toJson(tag));
    addASHIRTAuth(builder);
    return builder->execute(nam);
  }

  /// createOperation attempts to create a new operation with the given name and slug
  QNetworkReply *createOperation(QString name, QString slug) {
    auto builder = ashirtJSONPost("/api/operations", dto::Operation::createOperationJson(name, slug));
    addASHIRTAuth(builder);
    return builder->execute(nam);
  }

  /// extractResponse inspects the provided QNetworkReply and returns back the contents of the reply.
  /// In addition, it will also indicated, via the provided valid flag, if the response was valid.
  /// A Valid response is one that has a 200 or 201 response AND had no errors flaged from Qt
  /// Note: this applies to ASHIRT responses only
  static QByteArray extractResponse(QNetworkReply *reply, bool &valid) {
    auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    auto reqErr = reply->error();
    valid = (QNetworkReply::NoError == reqErr && status.isValid());
    valid = valid && (status == 200 || status == 201);
    return reply->readAll();
  }

  /// checkForNewRelease retrieves the recent releases from github for the provided owner/repo project.
  /// Callers should retrieve the result by listening for the releasesChecked signal
  void checkForNewRelease(QString owner, QString repo) {
    if (owner == Constants::unknownOwnerValue() || repo == Constants::unknownRepoValue()) {
      std::cerr << "Skipping release check: no owner or repo set." << std::endl;
      return;
    }
    githubReleaseReply = getGithubReleases(owner, repo);
    connect(githubReleaseReply, &QNetworkReply::finished, this, &NetMan::onGithubReleasesComplete);
  }

 private:
  QNetworkReply *allOpsReply = nullptr;
  QNetworkReply *githubReleaseReply = nullptr;
};

#endif  // NETMAN_H
