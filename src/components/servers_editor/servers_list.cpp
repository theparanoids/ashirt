// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "servers_list.h"

#include <QMessageBox>

#include "appservers.h"


ServersList::ServersList(QWidget* parent) : QWidget(parent) {
  buildUi();
  wireUi();
}

ServersList::~ServersList() {
  delete connectionsList;
  delete deleteButton;
  delete addButton;

  delete gridLayout;
}

void ServersList::buildUi() {
  gridLayout = new QGridLayout(this);
  gridLayout->setContentsMargins(0, 0, 0, 0);

  auto simpleButton = [this](QString name, int maxWidth = 0, int minWidth = 0){
    QPushButton* btn = new QPushButton(name, this);
    if (maxWidth > 0) {
      btn->setMaximumWidth(maxWidth);
      if(minWidth > 0) {
        btn->setMinimumWidth(minWidth);
      }
      btn->resize(maxWidth, btn->height());
    }
    btn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    return std::move(btn);
  };

  deleteButton = simpleButton("-", 40, 25);
  deleteButton->setEnabled(false); // start disabled, since no item is selected
  addButton = simpleButton("+", 40, 25);

  connectionsList = new QListWidget(this);
  connectionsList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  connectionsList->setSelectionMode(QAbstractItemView::ExtendedSelection);


  // Layout
  /*        0                 1
       +---------------+-------------+
       |                             |
    0  |   Connection List           |
       |                             |
       +---------------+-------------+
    1  | :Delete Btn   |  Add Btn:   |
       +---------------+-------------+
  */

  // row 0
  gridLayout->addWidget(connectionsList, 0, 0, 1, 2);

  // row 1
  gridLayout->addWidget(deleteButton, 1, 0);
  gridLayout->addWidget(addButton, 1, 1, Qt::AlignRight);

  this->setLayout(gridLayout);
}

void ServersList::wireUi() {
  auto btnClicked = &QPushButton::clicked;
  connect(addButton, btnClicked, this, &ServersList::addClicked);
  connect(deleteButton, btnClicked, this, &ServersList::deleteClicked);
  connect(connectionsList, &QListWidget::itemSelectionChanged, this, &ServersList::onItemSelectionChanged);
  connect(&AppSettings::getInstance(), &AppSettings::onServerUpdated, this, &ServersList::updateServerNames);
}

void ServersList::updateServerNames() {
  for (auto listItem : connectionsList->findItems("", Qt::MatchContains)) {
    auto serverItem = readItemData(listItem);
    listItem->setText(getServerName(serverItem));
  }
}

void ServersList::addClicked() {
  ServerItem s("New Connection", "", "", "");
  addMockServer(s);
  updateList();

  connectionsList->setCurrentRow(connectionsList->count() - 1);
  emit onServerAdded(s);
}

void ServersList::deleteClicked() {
  auto selectedItems = connectionsList->selectedItems();
  if (selectedItems.length() == 0) {
    return;
  }

  QString deleteMsg = "Are you sure you want to delete %1?";
  deleteMsg = deleteMsg.arg(selectedItems.length() == 1
                                ? "this server"
                                : QString("these %1 servers").arg(selectedItems.size()) );
  auto reply = QMessageBox::question(this, "Delete connections?", deleteMsg);
  if (reply != QMessageBox::Yes) {
    return;
  }

  std::vector<ServerItem> affectedServers;
  for(auto selectedItem : selectedItems) {
    auto data = readItemData(selectedItem);
    affectedServers.push_back(data);
    removeMockServer(data.getServerUuid());
  }
  updateList();
  emit onServersDeleted(affectedServers);
}

void ServersList::removeMockServer(QString uuid) {
  // this list should be relatively small, so this shouldn't really be that bad
  for (auto it = mockServers.begin(); it != mockServers.end(); it++) {
    auto server = it.base();
    if (server->getServerUuid() == uuid) {
      mockServers.erase(it);
      break;
    }
  }
}

void ServersList::addMockServer(ServerItem item) {
  mockServers.push_back(item);
}

void ServersList::onItemSelectionChanged() {
  auto selectedItems = connectionsList->selectedItems();
  std::vector<ServerItem> selectedServers;
  for (auto item : selectedItems) {
    auto data = readItemData(item);
    selectedServers.push_back(data);
  }

  deleteButton->setEnabled(selectedServers.size() > 0);
  emit onServerSelectionChanged(selectedServers);
}

void ServersList::repopulateTable() {
  mockServers = AppServers::getInstance().getServers();
  updateList();
}

void ServersList::updateList() {
  connectionsList->clear();

  for (auto server : mockServers) {
    auto item = buildServerItem(server);
    connectionsList->addItem(item);
  }
}

QString ServersList::getServerName(ServerItem server) {
  QString postfix = "";
  if (AppServers::getInstance().currentServerUuid() == server.getServerUuid()) {
    postfix = " (current)";
  }
  return server.serverName + postfix;
}

QListWidgetItem* ServersList::buildServerItem(ServerItem server) {
  auto item = new QListWidgetItem();
  auto castedData = QVariant::fromValue(server);
  item->setData(Qt::UserRole, castedData);
  item->setText(getServerName(server));

  if (server.deleted) {
    auto font = item->font();
    font.setStrikeOut(true);
    item->setFont(font);
  }

  return item;
}

void ServersList::clearSelection() {
  connectionsList->clearSelection();
}

void ServersList::refreshList() {
  repopulateTable();
}

void ServersList::saveServer(ServerItem serverItem) {
  for (auto it = mockServers.begin(); it != mockServers.end(); ++it) {
    if (it->getServerUuid() == serverItem.getServerUuid()) {
      *it = serverItem;
      break;
    }
  }
  for (auto listItem : connectionsList->findItems("", Qt::MatchContains)) {
    if (readItemData(listItem).getServerUuid() == serverItem.getServerUuid()) {
      listItem->setData(Qt::UserRole, QVariant::fromValue(serverItem));
      listItem->setText(getServerName(serverItem));
      break;
    }
  }
}

std::vector<ServerItem> ServersList::encodeServers() {
  return mockServers;
}

ServerItem ServersList::readItemData(QListWidgetItem* item) {
  return qvariant_cast<ServerItem>(item->data(Qt::UserRole));
}
