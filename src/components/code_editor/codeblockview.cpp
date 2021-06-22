#include "codeblockview.h"

#include "exceptions/fileerror.h"
#include "helpers/u_helpers.h"

CodeBlockView::CodeBlockView(QWidget* parent) : EvidencePreview(parent) {
  buildUi();
  wireUi();
}

CodeBlockView::~CodeBlockView() {
  delete _languageLabel;
  delete _sourceLabel;
  delete codeEditor;
  delete sourceTextBox;
  delete languageComboBox;
  delete gridLayout;
}

void CodeBlockView::buildUi() {
  gridLayout = new QGridLayout(this);
  gridLayout->setContentsMargins(0, 0, 0, 0);

  _languageLabel = new QLabel("Language", this);
  _sourceLabel = new QLabel("Source", this);
  sourceTextBox = new QLineEdit(this);
  languageComboBox = new QComboBox(this);
  codeEditor = new CodeEditor(this);

  for (const std::pair<QString, QString>& lang : SUPPORTED_LANGUAGES) {
    languageComboBox->addItem(lang.first, lang.second);
  }

  // Layout
  /*        0              1           2          3
       +------------+-------------+----------+----------------+
    0  | Lang Lab   | [ textbox ] | Src Lab  | [src textbox]  |
       +------------+-------------+----------+----------------+
    1  |                                                      |
       |            Code Block                                |
       |                                                      |
       +------------+-------------+----------+----------------+
  */

  // row 0
  gridLayout->addWidget(_languageLabel, 0, 0);
  gridLayout->addWidget(languageComboBox, 0, 1);
  gridLayout->addWidget(_sourceLabel, 0, 2);
  gridLayout->addWidget(sourceTextBox, 0, 3);

  // row 1
  gridLayout->addWidget(codeEditor, 1, 0, 1, gridLayout->columnCount());
}

void CodeBlockView::wireUi() {}

// ------- Parent Overrides

void CodeBlockView::loadFromFile(QString filepath) {
  try {
    loadedCodeblock = Codeblock::readCodeblock(filepath);

    codeEditor->setPlainText(loadedCodeblock.content);
    sourceTextBox->setText(loadedCodeblock.source);
    UiHelpers::setComboBoxValue(languageComboBox, loadedCodeblock.subtype);
  }
  catch (std::exception& e) {
    std::string msg = "Unable to load codeblock. Error: ";
    msg += e.what();
    codeEditor->setPlainText(QString(msg.c_str()));
    setReadonly(true);
  }
}

void CodeBlockView::saveEvidence() {
  loadedCodeblock.source = sourceTextBox->text();
  loadedCodeblock.subtype = languageComboBox->currentData().toString();
  loadedCodeblock.content = codeEditor->toPlainText();
  if (loadedCodeblock.filePath() != "") {
    try {
      Codeblock::saveCodeblock(loadedCodeblock);
    }
    catch (FileError& e) {
      // best effort here -- if we can't save, oh well, they can edit it on the server
    }
  }
}

void CodeBlockView::clearPreview() {
  codeEditor->setPlainText("");
  sourceTextBox->setText("");
  languageComboBox->setCurrentIndex(0);  // should be Plain Text
}

void CodeBlockView::setReadonly(bool readonly) {
  EvidencePreview::setReadonly(readonly);
  codeEditor->setReadOnly(readonly);
  sourceTextBox->setReadOnly(readonly);
  languageComboBox->setEnabled(!readonly);
}
