#ifndef SYSTEM_OPTIONS_H
#define SYSTEM_OPTIONS_H

namespace porting {

/**
 * @brief The SystemManifestExportOptions class provides a set of options for creating a SystemManifest
 * specifically for exporting
 */
class SystemManifestExportOptions {

 public:
  /// includesAnything is a simple function that checks if any export option is "true" (i.e. something that
  /// can be exported).
  bool includesAnything() const {
    return exportConfig || exportDb || exportServers;
  }

 public:
  /// exportConfig is a flag that denotes if the configuration file shall be exported. True = Yes, export. False = No, do not export
  bool exportConfig = true;

  /// exportServers is a flag that denotes if the servers information shall be exported. True = Yes, export. False = No, do not export
  bool exportServers = true;

  /// exportDb is a flag that denotes if the database (and evidence files) shall be exported. True = Yes, export. False = No, do not export
  bool exportDb = true;
};

/**
 * @brief The SystemManifestImportOptions class provides a set of options for creating a SystemManifest
 * specifically for importing
 */
class SystemManifestImportOptions {
 public:
  /// ImportAction represents the kind of import that can take place for evidence. The only options
  /// currently are Merge (yes, import) or None (no, don't import)
  enum ImportAction {
    /// None indicates to the importer that evidence SHOULD NOT be imported
    None = 0,
    /// Merge indicates to the importer that evidence SHOULD be imported
    Merge = 1,
    };

  /// includesAnything is a simple function that checks if any import option is set to add data
  /// (i.e. that there is work to be done)
  bool includesAnything() const {
    return importConfig || (importDb != None) || importServers;
  }

 public:
  /// importConfig is a flag that denotes if the configuration file shall be imported. True = Yes, import. False = No, do not import
  bool importConfig = true;

  /// importServers is a flag that denotes if the servers information shall be imported. True = Yes, import. False = No, do not import
  bool importServers = true;

  /// importDb is an ImportAction that determines HOW importing should proceed.
  ImportAction importDb = Merge;
};

}

#endif // SYSTEM_OPTIONS_H
