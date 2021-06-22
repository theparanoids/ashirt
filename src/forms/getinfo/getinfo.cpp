// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "getinfo.h"

#include <QKeySequence>
#include <QMessageBox>

#include "appsettings.h"
#include "components/evidence_editor/evidenceeditor.h"
#include "helpers/netman.h"
#include "helpers/stopreply.h"
#include "helpers/u_helpers.h"

GetInfo::GetInfo(DatabaseConnection* db, qint64 evidenceID, QWidget* parent)
    : QDialog(parent), db(db), evidenceID(evidenceID) {
  this->db = db;
  this->evidenceID = evidenceID;

  buildUi();
  wireUi();
}

GetInfo::~GetInfo() {
  delete evidenceEditor;
  delete submitButton;
  delete deleteButton;
  delete closeWindowAction;

  delete gridLayout;
  stopReply(&uploadAssetReply);
}

void GetInfo::buildUi() {
  gridLayout = new QGridLayout(this);

  submitButton = new LoadingButton("Submit", this);
  submitButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  submitButton->setAutoDefault(false);
  deleteButton = new QPushButton("Delete", this);
  deleteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  deleteButton->setAutoDefault(false);

  evidenceEditor = new EvidenceEditor(evidenceID, db, this);
  evidenceEditor->setEnabled(true);

  // Layout
  /*        0                 1            2
       +---------------+-------------+------------+
    0  |                                          |
       |             Evidence Editor              |
       |                                          |
       +---------------+-------------+------------+
    1  | Delete Btn    | <None>      | Submit Btn |
       +---------------+-------------+------------+
  */

  // row 0
  gridLayout->addWidget(evidenceEditor, 0, 0, 1, 3);

  // row 1
  gridLayout->addWidget(deleteButton, 1, 0);
  gridLayout->addWidget(submitButton, 1, 2);

  closeWindowAction = new QAction(this);
  closeWindowAction->setShortcut(QKeySequence::Close);
  this->addAction(closeWindowAction);

  this->setLayout(gridLayout);
  this->setAttribute(Qt::WA_DeleteOnClose);
  this->resize(720, 480);
  this->setWindowTitle("Add Evidence Details");

  // Make the dialog pop up above any other windows but retain title bar and buttons
  Qt::WindowFlags flags = this->windowFlags();
  flags |= Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::WindowMinMaxButtonsHint |
           Qt::WindowCloseButtonHint;
  this->setWindowFlags(flags);
  setFocus(); // ensure focus is not on the submit button
}

void GetInfo::wireUi() {
  connect(submitButton, &QPushButton::clicked, this, &GetInfo::submitButtonClicked);
  connect(deleteButton, &QPushButton::clicked, this, &GetInfo::deleteButtonClicked);
  connect(closeWindowAction, &QAction::triggered, this, &GetInfo::deleteButtonClicked);
}

void GetInfo::showEvent(QShowEvent* evt) {
  QDialog::showEvent(evt);
  setFocus();  // giving the form focus, to prevent retaining focus on the submit button when
               // closing the window
}

bool GetInfo::saveData() {
  auto saveResponse = evidenceEditor->saveEvidence();
  if (!saveResponse.actionSucceeded) {
    QMessageBox::warning(this, "Cannot Save",
                         "Unable to save evidence data.\n"
                         "You can try uploading directly to the website. File Location:\n" +
                             saveResponse.model.path);
  }
  return saveResponse.actionSucceeded;
}

void GetInfo::submitButtonClicked() {
  submitButton->startAnimation();
  setActionButtonsEnabled(false);
  if (saveData()) {
    try {
      model::Evidence evi = db->getEvidenceDetails(evidenceID);
      uploadAssetReply = NetMan::getInstance().uploadAsset(evi);
      connect(uploadAssetReply, &QNetworkReply::finished, this, &GetInfo::onUploadComplete);
    }
    catch (QSqlError& e) {
      QMessageBox::warning(this, "Cannot submit evidence",
                           "Could not retrieve data. Please try again.");
    }
  }
}

void GetInfo::deleteButtonClicked() {
  auto reply = QMessageBox::question(this, "Discard Evidence",
                                     "Are you sure you want to discard this evidence?",
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (reply == QMessageBox::Yes) {
    setActionButtonsEnabled(false);
    bool shouldClose = true;

    model::Evidence evi = evidenceEditor->encodeEvidence();
    if (!QFile::remove(evi.path)) {
      QMessageBox::warning(this, "Could not delete",
                           "Unable to delete evidence file.\n"
                           "You can try deleting the file directly. File Location:\n" +
                               evi.path);
      shouldClose = false;
    }
    try {
      db->deleteEvidence(evidenceID);
    }
    catch (QSqlError& e) {
      std::cout << "Could not delete evidence from internal database. Error: "
                << e.text().toStdString() << std::endl;
    }

    setActionButtonsEnabled(true);
    if (shouldClose) {
      this->close();
    }
  }
}

void GetInfo::setActionButtonsEnabled(bool enabled) {
  submitButton->setEnabled(enabled);
  deleteButton->setEnabled(enabled);
}

void GetInfo::onUploadComplete() {
  if (uploadAssetReply->error() != QNetworkReply::NoError) {
    auto errMessage =
        "Unable to upload evidence: Network error (" + uploadAssetReply->errorString() + ")";
    try {
      db->updateEvidenceError(errMessage, this->evidenceID);
    }
    catch (QSqlError& e) {
      std::cout << "Upload failed. Could not update internal database. Error: "
                << e.text().toStdString() << std::endl;
    }
    QMessageBox::warning(this, "Cannot submit evidence",
                         "Upload failed: Network error. Check your connection and try again.\n"
                         "Note: This evidence has been saved. You can close this window and "
                         "re-submit from the evidence manager."
                         "\n(Error: " +
                             uploadAssetReply->errorString() + ")");
  }
  else {
    try {
      db->updateEvidenceSubmitted(this->evidenceID);
      emit evidenceSubmitted(db->getEvidenceDetails(this->evidenceID));
      this->close();
    }
    catch (QSqlError& e) {
      std::cout << "Upload successful. Could not update internal database. Error: "
                << e.text().toStdString() << std::endl;
    }
  }
  // we don't actually need anything from the uploadAssets reply, so just clean it up.
  // one thing we might want to record: evidence uuid... not sure why we'd need it though.
  submitButton->stopAnimation();
  setActionButtonsEnabled(true);
  tidyReply(&uploadAssetReply);
}
