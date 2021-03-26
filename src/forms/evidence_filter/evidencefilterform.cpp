// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "evidencefilterform.h"

#include <QKeySequence>

#include "appservers.h"
#include "appsettings.h"
#include "helpers/netman.h"
#include "helpers/ui_helpers.h"

static void initializeTriCombobox(QComboBox *box) {
  box->clear();
  box->addItem("Any");
  box->addItem("Yes");
  box->addItem("No");
}

static void initializeDateEdit(QDateEdit *dateEdit) {
  dateEdit->setDate(QDateTime::currentDateTime().date());
  dateEdit->setDisplayFormat("MMM dd, yyyy");
  dateEdit->setDateRange(QDate(2000, 01, 01), QDateTime::currentDateTime().date());
  dateEdit->setEnabled(false);
}

EvidenceFilterForm::EvidenceFilterForm(DatabaseConnection* db, QWidget *parent)
    : QDialog(parent) {
  this->db = db;
  buildUi();
  wireUi();
}

EvidenceFilterForm::~EvidenceFilterForm() {
  delete closeWindowAction;

  delete _operationLabel;
  delete _contentTypeLabel;
  delete _hadErrorLabel;
  delete _wasSubmittedLabel;
  delete _fromDateLabel;
  delete _toDateLabel;
  delete _severLabel;

  delete operationComboBox;
  delete serverComboBox;
  delete submittedComboBox;
  delete erroredComboBox;
  delete contentTypeComboBox;
  delete fromDateEdit;
  delete toDateEdit;
  delete includeEndDateCheckBox;
  delete includeStartDateCheckBox;
  delete buttonBox;

  delete gridLayout;
}

void EvidenceFilterForm::buildUi() {
  gridLayout = new QGridLayout(this);

  _severLabel = new QLabel("Server", this);
  _operationLabel = new QLabel("Operation", this);
  _contentTypeLabel = new QLabel("Content Type", this);
  _hadErrorLabel = new QLabel("Had Error", this);
  _wasSubmittedLabel = new QLabel("Was Submitted", this);
  _fromDateLabel = new QLabel("From Date", this);
  _toDateLabel = new QLabel("To Date", this);

  operationComboBox = new QComboBox(this);
  operationComboBox->setEditable(false);
  operationComboBox->setEnabled(false);
  operationComboBox->addItem("Loading...", "");

  serverComboBox = new QComboBox(this);
  serverComboBox->setEditable(false);
  populateServerComboBox();

  submittedComboBox = new QComboBox(this);
  submittedComboBox->setEditable(false);
  initializeTriCombobox(submittedComboBox);

  erroredComboBox = new QComboBox(this);
  erroredComboBox->setEditable(false);
  initializeTriCombobox(erroredComboBox);

  contentTypeComboBox = new QComboBox(this);
  contentTypeComboBox->setEditable(false);
  contentTypeComboBox->addItem("<Any>", "");
  contentTypeComboBox->addItem("Image", "image");
  contentTypeComboBox->addItem("Codeblock", "codeblock");

  fromDateEdit = new QDateEdit(this);
  initializeDateEdit(fromDateEdit);
  toDateEdit = new QDateEdit(this);
  initializeDateEdit(toDateEdit);

  includeEndDateCheckBox = new QCheckBox("Include", this);
  includeStartDateCheckBox = new QCheckBox("Include", this);

  buttonBox = new QDialogButtonBox(this);
  buttonBox->addButton(QDialogButtonBox::Ok);

  // Layout
  /*        0                 1           2
       +---------------+-------------+--------------+
    0  | Op  Lbl       | Op CB                      |
       +---------------+-------------+--------------+
    1  | Op  Lbl       | Op CB                      |
       +---------------+-------------+--------------+
    2  | C. Type Lbl   | Content Type CB            |
       +---------------+-------------+--------------+
    3  | hadErr Lbl    | had Error CB               |
       +---------------+-------------+--------------+
    4  | Submit Lbl    | Was Submitted CB           |
       +---------------+-------------+--------------+
    5  | From Lbl      | From DtSel  | incl From CB |
       +---------------+-------------+--------------+
    6  | To Lbl        | To DtSel    | incl To CB   |
       +---------------+-------------+--------------+
    7  | Dialog button Box{ok}                      |
       +---------------+-------------+--------------+
  */

  int row = 0;
  // row 0
  gridLayout->addWidget(_severLabel, row, 0);
  gridLayout->addWidget(serverComboBox, row, 1, 1, 2);
  row++;

  // row 0
  gridLayout->addWidget(_operationLabel, row, 0);
  gridLayout->addWidget(operationComboBox, row, 1, 1, 2);
  row++;

  // row 2
  gridLayout->addWidget(_contentTypeLabel, row, 0);
  gridLayout->addWidget(contentTypeComboBox, row, 1, 1, 2);
  row++;

  // row 3
  gridLayout->addWidget(_hadErrorLabel, row, 0);
  gridLayout->addWidget(erroredComboBox, row, 1, 1, 2);
  row++;

  // row 4
  gridLayout->addWidget(_wasSubmittedLabel, row, 0);
  gridLayout->addWidget(submittedComboBox, row, 1, 1, 2);
  row++;

  // row 5
  gridLayout->addWidget(_fromDateLabel, row, 0);
  gridLayout->addWidget(fromDateEdit, row, 1);
  gridLayout->addWidget(includeStartDateCheckBox, row, 2);
  row++;

  // row 6
  gridLayout->addWidget(_toDateLabel, row, 0);
  gridLayout->addWidget(toDateEdit, row, 1);
  gridLayout->addWidget(includeEndDateCheckBox, row, 2);
  row++;

  // row 7
  gridLayout->addWidget(buttonBox, row, 0, 1, gridLayout->columnCount());
  row++;

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  this->setLayout(gridLayout);
  this->setWindowTitle("Evidence Filters");
  this->resize(320, 245);
}

void EvidenceFilterForm::wireUi() {
  erroredComboBox->installEventFilter(this);
  operationComboBox->installEventFilter(this);
  serverComboBox->installEventFilter(this);
  submittedComboBox->installEventFilter(this);
  contentTypeComboBox->installEventFilter(this);

  connect(&NetMan::getInstance(), &NetMan::operationListUpdated, this,
          &EvidenceFilterForm::onOperationListUpdated);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &EvidenceFilterForm::writeAndClose);

  connect(includeStartDateCheckBox, &QCheckBox::stateChanged,
          [this](bool checked) { fromDateEdit->setEnabled(checked); });
  connect(includeEndDateCheckBox, &QCheckBox::stateChanged,
          [this](bool checked) { toDateEdit->setEnabled(checked); });

  connect(closeWindowAction, &QAction::triggered, this, &EvidenceFilterForm::writeAndClose);
}

void EvidenceFilterForm::resetOperations() {
  QString serverUuid = serverComboBox->currentData().toString();
  if(serverUuid == "") {
    updateOperationsList("", true, {});
  }
  else {
    auto foundServer = AppServers::getInstance().getServerByUuid(serverUuid);
    if (foundServer.isValid()) {
      operationComboBox->setEnabled(false);
      auto reply = NetMan::getInstance().getAllOperations(foundServer.hostPath, foundServer.accessKey, foundServer.secretKey);
      connect(reply, &QNetworkReply::finished, [reply, this, serverUuid](){
        bool success = false;
        // only update if the current uuid matches -- otherwise we might show the wrong list of operations for a server
        if(serverComboBox->currentData().toString() == serverUuid) {
          auto ops = NetMan::getInstance().parseOpsResponse(reply, success);
          updateOperationsList(serverUuid, success, ops);
        }
        reply->close();
        reply->deleteLater();
        operationComboBox->setEnabled(true);
      });
    }
  }
}

void EvidenceFilterForm::writeAndClose() {
  writeForm();
  close();
}

void EvidenceFilterForm::writeForm() {
  auto filter = encodeForm();
  emit evidenceSet(filter);
}

EvidenceFilters EvidenceFilterForm::encodeForm() {
  EvidenceFilters filter;

  filter.hasError = EvidenceFilters::parseTri(erroredComboBox->currentText());
  filter.submitted = EvidenceFilters::parseTri(submittedComboBox->currentText());
  filter.operationSlug = operationComboBox->currentData().toString();
  filter.setServer(serverComboBox->currentData().toString());
  filter.contentType = contentTypeComboBox->currentData().toString();

  // swap dates so smaller date is always "from" / after
  if (fromDateEdit->isEnabled() && toDateEdit->isEnabled() &&
      fromDateEdit->date() > toDateEdit->date()) {
    auto copy = fromDateEdit->date();
    fromDateEdit->setDate(toDateEdit->date());
    toDateEdit->setDate(copy);
  }

  if (includeStartDateCheckBox->isChecked()) {
    filter.startDate = fromDateEdit->date();
  }
  if (includeEndDateCheckBox->isChecked()) {
    filter.endDate = toDateEdit->date();
  }

  return filter;
}

void EvidenceFilterForm::setForm(const EvidenceFilters &model) {
  enableServerSelectionConnection(false);
  operationComboBox->setEnabled(true);
  UiHelpers::setComboBoxValue(serverComboBox, model.getServerUuid());
  UiHelpers::setComboBoxValue(operationComboBox, model.operationSlug);
  UiHelpers::setComboBoxValue(contentTypeComboBox, model.contentType);
  erroredComboBox->setCurrentText(EvidenceFilters::triToString(model.hasError));
  submittedComboBox->setCurrentText(EvidenceFilters::triToString(model.submitted));

  includeStartDateCheckBox->setChecked(model.startDate.isValid());
  fromDateEdit->setDate(model.startDate.isValid() ? model.startDate
                                                      : QDateTime::currentDateTime().date());

  includeEndDateCheckBox->setChecked(model.endDate.isValid());
  toDateEdit->setDate(model.endDate.isValid() ? model.endDate
                                                  : QDateTime::currentDateTime().date());

  // swap dates so smaller date is always "from" / after
  if (model.startDate.isValid() && model.endDate.isValid() &&
      fromDateEdit->date() > toDateEdit->date()) {
    auto copy = fromDateEdit->date();
    fromDateEdit->setDate(toDateEdit->date());
    toDateEdit->setDate(copy);
  }
  enableServerSelectionConnection(true);
}

void EvidenceFilterForm::enableServerSelectionConnection(bool enable) {
  if (enable) {
    connect(serverComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &EvidenceFilterForm::resetOperations);
  }
  else {
    disconnect(serverComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &EvidenceFilterForm::resetOperations);
  }
}

void EvidenceFilterForm::updateOperationsList(QString selectedServerUUID, bool success, const std::vector<dto::Operation> &operations) {
  operationComboBox->setEnabled(false);
  operationComboBox->clear();
  operationComboBox->addItem("<Any>", "");

  std::vector<QString> knownOperationSlugs;
  std::vector<QString> missingSlugs;
  if (selectedServerUUID != "") {
    knownOperationSlugs = db->operationSlugsForServer(selectedServerUUID);
  }

  auto addOrphanedSlug = [this](std::vector<QString> slugs){
    for (const auto &slug : slugs) {
      operationComboBox->addItem(slug + " (removed)", slug);
    }
  };

  if (!success) {
    operationComboBox->setItemText(0, "Unable to fetch operations");
    operationComboBox->setCurrentIndex(0);
    addOrphanedSlug(knownOperationSlugs);
    return;
  }

  for (const auto &op : operations) {
    operationComboBox->addItem(op.name, op.slug);
    //if(!knownOperationSlugs.contains(op.slug)) {
  }
  for (const auto &slug : knownOperationSlugs) {
    bool found = false;
    for(const auto &op : operations) {
      if (slug == op.slug) {
        found = true;
        break;
      }
    }
    if (!found) {
      missingSlugs.push_back(slug);
    }
  }
  addOrphanedSlug(missingSlugs);

  UiHelpers::setComboBoxValue(operationComboBox, AppSettings::getInstance().operationSlug());
  operationComboBox->setEnabled(true);
}

void EvidenceFilterForm::onOperationListUpdated(bool success,
                                                const std::vector<dto::Operation> &operations) {

  updateOperationsList(AppServers::getInstance().currentServerUuid(), success, operations);
}

void EvidenceFilterForm::populateServerComboBox() {
  serverComboBox->addItem("<Any>", "");
  try {
    std::vector<ServerItem> servers = AppServers::getInstance().getServers(true);

    for (auto s : servers) {
      serverComboBox->addItem(s.serverName, s.getServerUuid());
    }
  }
  catch(const std::exception& e) {
    std::cerr << "Unable to get server details: " << e.what() << std::endl;
  }
}
