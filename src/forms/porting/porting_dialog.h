#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QAction>
#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QWidget>

#include "db/databaseconnection.h"
#include "porting/system_manifest.h"

/**
 * @brief The PortingDialog class renders a dialog window allowing a user to import or export content and settings.
 * The UI presented is dependent on the underlying dialogType. Specifying Import will render an import window,
 * while Export will render an export window.
 * @see PortType
 */
class PortingDialog : public QDialog {
  Q_OBJECT

 public:
  /// PortType is a small enum indicating if the PortingDialog renders an import or export window
  enum PortType {
    /// Import denotes that all actions and dialog options should be rendered for import purposes
    Import,
    /// Export denotes that all actions and dialog options should be rendered for export purposes
    Export
  };

 public:
  /**
   * @brief PortingDialog constructs a new Dialog window. After this method is called, the UI will be rendered and wired.
   * @param dialogType controls whether the rendered dialog window is used for Import or Export purposes
   * @param db is a reference to the standard, running database.
   * @param parent is used by the underlying QDialog constructor
   */
  explicit PortingDialog(PortType dialogType, DatabaseConnection* db, QWidget *parent = nullptr);
  ~PortingDialog();

 public:
  /// getPortPath retrieves the path used to import or export (note: this is always directory, even
  /// when Importing from a specific file)
  QString getPortPath();

 signals:
  /// portCompleted is called when an import or export finishes.
  /// @param path contains the import or export directory used
  void portCompleted(QString path);

 private:
  /// buildUi creates the window structure.
  void buildUi();
  /// wireUi connects the components to each other.
  void wireUi();

  /// onSubmitPressed preps an import/export and routes the action to doImport or doExport
  void onSubmitPressed();
  /// onBrowsePressed renders a QFileDialog window (for opening). The behavior is specific for
  /// Import and Export dialogs
  void onBrowsePresed();

  /// resetForm places the form back to it's original configuration.
  void resetForm();

  /// doExport begins the export process with the given System manifest and export path
  void doExport(porting::SystemManifest* manifest, const QString& exportPath);

  /// doPreImport opens and parses the provided path to the system manifest. A small amount of
  /// validation occurs. If any errors are encountered, then the import is cancelled
  porting::SystemManifest* doPreImport(const QString& pathToSystemManifest);
  /// doImport begins the import process with the given system manifest
  void doImport(porting::SystemManifest* manifest);

  /// onPortComplete is a common post-port action/cleanup for both import and export.
  void onPortComplete(bool success);

 signals:
  /// onWorkComplete is called as the final step in import/export. This signals to the dialog that
  /// onPortComplete can be called.
  void onWorkComplete(bool success);

 private:
  QString portPath = "";
  PortType dialogType;
  bool portDone = false;

  DatabaseConnection* db; // borrowed

  /// executedManifest contains a pointer to the system manifest used to import/export data
  /// Saved so that it can be cleaned up post-execution
  porting::SystemManifest* executedManifest = nullptr;

  QAction* closeWindowAction = nullptr;

  // UI Components
  QGridLayout* gridLayout = nullptr;
  QLabel* _selectFileLabel = nullptr;
  QLabel* portStatusLabel = nullptr;

  QPushButton* submitButton = nullptr;
  QPushButton* browseButton = nullptr;
  QLineEdit* pathTextBox = nullptr;
  QProgressBar* progressBar = nullptr;
  QCheckBox* portConfigCheckBox = nullptr;
  QCheckBox* portServersCheckBox = nullptr;
  QCheckBox* portEvidenceCheckBox = nullptr;
};

#endif // IMPORTDIALOG_H
