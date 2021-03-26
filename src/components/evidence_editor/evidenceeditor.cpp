// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#include "evidenceeditor.h"

#include <QFile>
#include <vector>

#include "components/aspectratio_pixmap_label/imageview.h"
#include "components/code_editor/codeblockview.h"
#include "components/error_view/errorview.h"
#include "components/evidence_editor/evidenceeditor.h"
#include "models/codeblock.h"
#include "models/evidence.h"

EvidenceEditor::EvidenceEditor(qint64 evidenceID, DatabaseConnection *db, QWidget *parent)
    : EvidenceEditor(db, parent) {
  this->evidenceID = evidenceID;
  loadData();
}

EvidenceEditor::EvidenceEditor(DatabaseConnection *db, QWidget *parent) : QWidget(parent) {
  buildUi();
  this->db = db;

  wireUi();
  setEnabled(false);
}

EvidenceEditor::~EvidenceEditor() {
  delete _descriptionLabel;
  delete descriptionTextBox;
  delete descriptionAreaLayout;
  delete descriptionArea;

  delete loadedPreview;
  delete splitter;
  delete tagEditor;

  delete gridLayout;
}

void EvidenceEditor::buildUi() {
  gridLayout = new QGridLayout(this);
  gridLayout->setContentsMargins(0, 0, 0, 0);

  splitter = new QSplitter(this);
  splitter->setOrientation(Qt::Vertical);
  splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  _descriptionLabel = new QLabel("Description", this);
  tagEditor = new TagEditor(this);
  descriptionTextBox = new QTextEdit(this);

  descriptionAreaLayout = new QVBoxLayout();
  descriptionArea = new QWidget(this);
  descriptionArea->setLayout(descriptionAreaLayout);
  descriptionAreaLayout->addWidget(_descriptionLabel);
  descriptionAreaLayout->addWidget(descriptionTextBox);
  descriptionAreaLayout->setContentsMargins(0, 0, 0, 0);

  // Layout
  /*        0
       +----------------------------------------+
    0  | +------------Vert. Splitter---------+  |
       | |                                   |  |
       | |        Preview Area (reserved)    |  |
       | +-----------------------------------+  |
       | |  +-----------------------------+  |  |
       | |  | description label           |  |  |
       | |  +-----------------------------|  |  |
       | |  |       Desc Text box         |  |  |
       | |  |        (minimized)          |  |  |
       | |  +-----------------------------|  |  |
       | +-----------------------------------+  |
       +----------------------------------------+
    1  |                                        |
       |                 tagEditor              |
       |                                        |
       +----------------------------------------+
  */

  // row 0
  gridLayout->addWidget(splitter, 0, 0);
  splitter->addWidget(descriptionArea);

  // row 1
  gridLayout->addWidget(tagEditor, 1, 0);

  this->setLayout(gridLayout);
}

model::Evidence EvidenceEditor::encodeEvidence() {
  model::Evidence copy = model::Evidence(originalEvidenceData);

  copy.description = descriptionTextBox->toPlainText();
  copy.tags.clear();
  copy.tags = tagEditor->getIncludedTags();

  return copy;
}

void EvidenceEditor::setEnabled(bool enable) {
  // if the product is enabled, then we can edit, hence it's not readonly
  descriptionTextBox->setReadOnly(!enable);
  tagEditor->setReadonly(!enable);
  if (loadedPreview != nullptr) {
    loadedPreview->setReadonly(!enable);
  }
}

void EvidenceEditor::wireUi() {
  connect(tagEditor, &TagEditor::tagsLoaded, this, &EvidenceEditor::onTagsLoaded);
}

void EvidenceEditor::loadData() {
  // get local db evidence data
  clearEditor();
  delete loadedPreview;
  try {
    originalEvidenceData = db->getEvidenceDetails(evidenceID);
    descriptionTextBox->setText(originalEvidenceData.description);
    operationSlug = originalEvidenceData.operationSlug;

    if (originalEvidenceData.contentType == "image") {
      loadedPreview = new ImageView(this);
      loadedPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    else if (originalEvidenceData.contentType == "codeblock") {
      loadedPreview = new CodeBlockView(this);
      loadedPreview->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }
    else {
      loadedPreview =
          new ErrorView("Unsupported evidence type: " + originalEvidenceData.contentType, this);
    }
    loadedPreview->loadFromFile(originalEvidenceData.path);
    loadedPreview->setReadonly(readonly);

    // get all remote tags (for op)
    tagEditor->loadTags(operationSlug, originalEvidenceData.tags);
  }
  catch (QSqlError &e) {
    loadedPreview = new ErrorView("Unable to load evidence: " + e.text(), this);
  }
  splitter->insertWidget(0, loadedPreview);
}

void EvidenceEditor::updateEvidence(qint64 evidenceID, bool readonly) {
  clearEditor();
  setEnabled(false);
  this->readonly = readonly;
  this->evidenceID = evidenceID;
  if (evidenceID > 0) {
    loadData();
  }
}

void EvidenceEditor::clearEditor() {
  tagEditor->clear();
  this->descriptionTextBox->setText("");
  if (loadedPreview != nullptr) {
    loadedPreview->clearPreview();
    delete loadedPreview;
    loadedPreview = nullptr;
  }
}

void EvidenceEditor::onTagsLoaded(bool success) {
  tagEditor->setReadonly(!success || readonly);
  emit onWidgetReady();
}

// saveEvidence is a helper method to save (to the database) the currently
// loaded evidence, using the editor changes.
SaveEvidenceResponse EvidenceEditor::saveEvidence() {
  if (loadedPreview != nullptr) {
    loadedPreview->saveEvidence();
  }
  auto evi = encodeEvidence();
  auto resp = SaveEvidenceResponse(evi);
  try {
    db->updateEvidenceDescription(evi.description, evi.id);
    db->setEvidenceTags(evi.tags, evi.id);
    resp.actionSucceeded = true;
  }
  catch (QSqlError &e) {
    resp.actionSucceeded = false;
    resp.errorText = e.text();
  }
  return resp;
}

std::vector<DeleteEvidenceResponse> EvidenceEditor::deleteEvidence(std::vector<qint64> evidenceIDs) {
  std::vector<DeleteEvidenceResponse> responses;

  for (qint64 id : evidenceIDs) {
    model::Evidence evi = db->getEvidenceDetails(id);
    DeleteEvidenceResponse resp(evi);
    try {
      db->deleteEvidence(evi.id);
      resp.dbDeleteSuccess = true;
    }
    catch (QSqlError &e) {
      resp.dbDeleteSuccess = false;
      resp.errorText = e.text();
    }
    auto localFile = new QFile(evi.path);
    if (!localFile->remove()) {
      resp.fileDeleteSuccess = false;
      resp.errorText += "\n" + localFile->errorString();
    }
    else {
      resp.fileDeleteSuccess = true;
    }
    localFile->deleteLater(); // deletes the pointer, not the file
    resp.errorText = resp.errorText.trimmed();
    responses.push_back(resp);
  }
  
  return responses;
}
