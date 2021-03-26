// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef SERVERS_LIST_H
#define SERVERS_LIST_H

#include <QGridLayout>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QWidget>

#include "config/server_item.h"

/**
 * @brief The ServersList class is a widget for displaying a list of servers currently known to the
 * application. It also allows for new servers to be added, or deleted. These changes do not
 * affect the system's state, but rather
 */
class ServersList : public QWidget {
  Q_OBJECT

 public:
  ServersList(QWidget* parent = nullptr);
  ~ServersList();

 private:
  /// buildUi constructs the window structure.
  void buildUi();
  /// wireUi connects UI elements together
  void wireUi();

  /// repopulateTable pulls data from systems and replaces the internal representation with these
  /// values
  void repopulateTable();

  /// updateList updates the connectionsList to reflect the current state of the
  /// internal representation of servers. (note that this removes all changes currently in memory)
  void updateList();

  /// buildServerItem constructs a QListWidgetItem from a given server, as a model. The item label
  /// is the serverName, while the item user data is ServerItem itself
  QListWidgetItem* buildServerItem(ServerItem server);

  ServerItem readItemData(QListWidgetItem* item);

  QString getServerName(ServerItem server);

 public:
  void clearSelection();
  void refreshList();
  std::vector<ServerItem> encodeServers();

 signals:
  void onServerAdded(ServerItem s);
  void onServersDeleted(std::vector<ServerItem>);
  void onServerSelectionChanged(std::vector<ServerItem>);

 public slots:
  void saveServer(ServerItem serverItem);

 private slots:
  void addClicked();
  void deleteClicked();
  void onItemSelectionChanged();
  void addMockServer(ServerItem item);
  void removeMockServer(QString uuid);
  void updateServerNames();

 private:
  std::vector<ServerItem> mockServers;

  // UI Elements
  QGridLayout* gridLayout = nullptr;

  QListWidget* connectionsList = nullptr;
  QPushButton* addButton = nullptr;
  QPushButton* deleteButton = nullptr;
};

#endif // SERVERS_LIST_H
