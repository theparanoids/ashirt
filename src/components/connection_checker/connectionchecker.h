#ifndef CONNECTIONCHECKER_H
#define CONNECTIONCHECKER_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QNetworkReply>


#include "components/loading_button/loadingbutton.h"

class ConnectionChecker : public QWidget {
  Q_OBJECT
 public:
  explicit ConnectionChecker(bool useAltLayout=false, QWidget *parent = nullptr);
  ~ConnectionChecker();

 signals:
  /// pressed is emitted when
  void pressed();
  /// started is emitted when
  void started();
  /// completed is emitted when
  void completed();

 private:
  void buildUi(bool useAltLayout);
  void wireUi();

  ///
  void onTestConnectionClicked();
  ///
  void onTestRequestComplete();


 public:
  void setConnectionTestFields(QString hostPath, QString accessKey, QString secretKey);
  void clearStatus();
  void abortRequest();

 private:
  QString hostPath;
  QString accessKey;
  QString secretKey;

  QNetworkReply* currentTestReply = nullptr;


  // Ui Elements
  QGridLayout* gridLayout = nullptr;
  LoadingButton* testConnButton = nullptr;
  QLabel* connStatusLabel = nullptr;
};

#endif // CONNECTIONCHECKER_H
