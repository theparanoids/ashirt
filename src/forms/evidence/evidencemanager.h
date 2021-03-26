// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef EVIDENCEMANAGER_H
#define EVIDENCEMANAGER_H

#include <QAction>
#include <QDialog>
#include <QLineEdit>
#include <QMenu>
#include <QNetworkReply>
#include <QTableWidget>
#include <QTableWidgetItem>

#include "components/evidence_editor/evidenceeditor.h"
#include "components/loading/qprogressindicator.h"
#include "db/databaseconnection.h"
#include "forms/evidence_filter/evidencefilterform.h"

/// EvidenceRow contains the necessary data for a full row in the evidence table.
/// QTableWidget should memory-manage this data.
struct EvidenceRow {
  QTableWidgetItem* dateCaptured;
  QTableWidgetItem* description;
  QTableWidgetItem* contentType;
  QTableWidgetItem* operation;
  QTableWidgetItem* submitted;
  QTableWidgetItem* failed;
  QTableWidgetItem* path;
  QTableWidgetItem* errorText;
  QTableWidgetItem* dateSubmitted;
  QTableWidgetItem* server;
};

/**
 * @brief The EvidenceManager class represents the Evidence Manager window that is shown
 * when selecting "View Accumulated Evidence."
 */
class EvidenceManager : public QDialog {
  Q_OBJECT

 public:
  explicit EvidenceManager(DatabaseConnection* db, QWidget* parent = nullptr);
  ~EvidenceManager();

 private:
  /// buildUi constructs the window structure.
  void buildUi();
  /// buildEvidenceTableUi constructs the evidence table.
  void buildEvidenceTableUi();

  /// wireUi connects UI elements together
  void wireUi();
  /// openTableContextMenu opens a context menu over the evidenceTable when right-clicking
  void openTableContextMenu(QPoint pos);

  /// saveData stores any edits in evidence view. Deprecated (edits no longer available)
  bool saveData();
  /// loadEvidence retrieves data from the database and renders the evidence table
  void loadEvidence();
  /// buildBaseEvidenceRow constructs a basic evidence row (fields and formatting, no data applied)
  EvidenceRow buildBaseEvidenceRow(qint64 evidenceID);
  /// refreshRow updates the indicated row (0-based) with updated (database) data.
  void refreshRow(int row);
  /// setRowText writes data the indicated row (0-based) based on the given model
  void setRowText(int row, const model::Evidence& model);

  /// showEvent extends QDialog's showEvent. Resets the applied filters.
  void showEvent(QShowEvent* evt) override;
  /// selectedRowEvidenceID is a small helper to get the evidence id for the currently selected row.
  qint64 selectedRowEvidenceID();
  /// selectedRowEvidenceIDs is a small helper to retrieve the id for all the selected rows
  std::vector<qint64> selectedRowEvidenceIDs();

 signals:
  /**
   * @brief evidenceChanged is emitted when a user changes the selection in the evidence table
   * @param evidenceID the evidence ID of the now-selected evidence
   * @param readonly True if this evidence can be edited. False otherwise.
   */
  void evidenceChanged(quint64 evidenceID, bool readonly);

 private slots:
  /// submitEvidenceTriggered recieves the triggered event from the submit action
  void submitEvidenceTriggered();
  /// deleteEvidenceTriggered recieves the triggered event from the delete action
  void deleteEvidenceTriggered();
  /// resetFilterButtonClicked recieves the reset filter button clicked event
  void resetFilterButtonClicked();
  /// deleteAllTriggered recieves the triggered event from the delete table action
  void deleteAllTriggered();
  
  /// deleteSet is a small helper to iterate through the provided list, delete the ids, and process
  /// the result
  void deleteSet(std::vector<qint64> ids);

  /// applyFilterForm updates the filter textbox to reflect the filter options chosen in the filter
  /// menu
  void applyFilterForm(const EvidenceFilters& filter);
  /// openFiltersMenu opens the filter menu with the current filters applied
  void openFiltersMenu();

  /// onRowChanged recieves the event from the evidence table rowChange signal
  void onRowChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
  /// onUploadComplete is triggered when the upload response has been received.
  void onUploadComplete();

  /// copyPathTriggered recives the triggered event from the copyPathToClipboardAction
  void copyPathTriggered();

 private:
  /// db is a (shared) reference to the local database instance. Not to be deleted.
  DatabaseConnection* db;

  QNetworkReply* uploadAssetReply = nullptr;
  qint64 evidenceIDForRequest = 0;

  // Subwindows
  EvidenceFilterForm* filterForm = nullptr;
  QMenu* evidenceTableContextMenu = nullptr;

  QAction* submitEvidenceAction = nullptr;
  QAction* deleteEvidenceAction = nullptr;
  QAction* closeWindowAction = nullptr;
  QAction* copyPathToClipboardAction = nullptr;
  QAction* deleteTableContentsAction = nullptr;

  // UI Elements
  QGridLayout* gridLayout = nullptr;
  QPushButton* editFiltersButton = nullptr;
  QPushButton* applyFilterButton = nullptr;
  QPushButton* resetFilterButton = nullptr;
  QLineEdit* filterTextBox = nullptr;
  QTableWidget* evidenceTable = nullptr;
  EvidenceEditor* evidenceEditor = nullptr;
  QProgressIndicator* loadingAnimation = nullptr;
  QSpacerItem* spacer = nullptr;
};

#endif  // EVIDENCEMANAGER_H
