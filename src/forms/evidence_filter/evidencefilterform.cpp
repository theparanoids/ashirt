// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "evidencefilterform.h"

#include <QKeySequence>

#include "appsettings.h"
#include "helpers/netman.h"
#include "helpers/u_helpers.h"

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

EvidenceFilterForm::EvidenceFilterForm(QWidget *parent)
    : QDialog(parent) {
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

  delete operationComboBox;
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

  submittedComboBox = new QComboBox(this);
  submittedComboBox->setEditable(false);
  initializeTriCombobox(submittedComboBox);

  erroredComboBox = new QComboBox(this);
  erroredComboBox->setEditable(false);
  initializeTriCombobox(erroredComboBox);

  contentTypeComboBox = new QComboBox(this);
  contentTypeComboBox->setEditable(false);
  contentTypeComboBox->addItem("<None>", "");
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
    1  | C. Type Lbl   | Content Type CB            |
       +---------------+-------------+--------------+
    2  | hadErr Lbl    | had Error CB               |
       +---------------+-------------+--------------+
    3  | Submit Lbl    | Was Submitted CB           |
       +---------------+-------------+--------------+
    4  | From Lbl      | From DtSel  | incl From CB |
       +---------------+-------------+--------------+
    5  | To Lbl        | To DtSel    | incl To CB   |
       +---------------+-------------+--------------+
    6  | Dialog button Box{ok}                      |
       +---------------+-------------+--------------+
  */

  // row 0
  gridLayout->addWidget(_operationLabel, 0, 0);
  gridLayout->addWidget(operationComboBox, 0, 1, 1, 2);

  // row 1
  gridLayout->addWidget(_contentTypeLabel, 1, 0);
  gridLayout->addWidget(contentTypeComboBox, 1, 1, 1, 2);

  // row 2
  gridLayout->addWidget(_hadErrorLabel, 2, 0);
  gridLayout->addWidget(erroredComboBox, 2, 1, 1, 2);

  // row 3
  gridLayout->addWidget(_wasSubmittedLabel, 3, 0);
  gridLayout->addWidget(submittedComboBox, 3, 1, 1, 2);

  // row 4
  gridLayout->addWidget(_fromDateLabel, 4, 0);
  gridLayout->addWidget(fromDateEdit, 4, 1);
  gridLayout->addWidget(includeStartDateCheckBox, 4, 2);

  // row 5
  gridLayout->addWidget(_toDateLabel, 5, 0);
  gridLayout->addWidget(toDateEdit, 5, 1);
  gridLayout->addWidget(includeEndDateCheckBox, 5, 2);

  // row 6
  gridLayout->addWidget(buttonBox, 6, 0, 1, gridLayout->columnCount());

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
}

void EvidenceFilterForm::onOperationListUpdated(bool success,
                                                const std::vector<dto::Operation> &operations) {
  operationComboBox->setEnabled(false);
  if (!success) {
    operationComboBox->setItemText(0, "Unable to fetch operations");
    operationComboBox->setCurrentIndex(0);
    return;
  }

  operationComboBox->clear();
  operationComboBox->addItem("<None>", "");
  for (const auto &op : operations) {
    operationComboBox->addItem(op.name, op.slug);
  }
  UiHelpers::setComboBoxValue(operationComboBox, AppSettings::getInstance().operationSlug());
  operationComboBox->setEnabled(true);
}
