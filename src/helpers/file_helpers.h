#ifndef FILE_HELPERS_H
#define FILE_HELPERS_H

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QRandomGenerator>
#include <QString>
#include <QStringList>
#include <array>

#include "exceptions/fileerror.h"

class FileCopyResult {
 public:
  FileCopyResult(){}
  bool success = false;
  QFile* file;
};


class FileReadResult {
 public:
  FileReadResult() = default;

  bool success = false;
  bool fileExists = false;
  QByteArray data;
  QFileDevice::FileError err;
};

class FileHelpers {
 public:
  /// randomText generates an arbitrary number of case-sensitive english letters
  static QString randomText(unsigned int numChars) {
    std::array<int, 2> asciiOffset = {'A', 'a'};
    QStringList replacement;

    for (unsigned int i = 0; i < numChars; i++) {
      int letter = QRandomGenerator::global()->bounded(52);
      auto base = asciiOffset.at(letter < 26 ? 0 : 1);
      replacement << QString(char(base + (letter % 26)));
    }
    return replacement.join("");
  }

  /**
   * @brief randomFilename replaces a given substring with random english letters. By default, the
   * expected substring is 6 consecutive "X" characters (i.e. XXXXXX). Users may specify their own
   * replacement string. Note that only the _first_ matching substring is replaced. If the
   * replaceToken is not found, then the templateString is returned, unmodified.
   * Each replacement letter may either be upper or lower case.
   * Similar to what QTemporaryFile does.
   * @param templateStr The model string, with
   * @param replaceToken The template string to find, and then replace, with random characters
   * @return The resulting filename
   */
  static QString randomFilename(QString templateStr, QString replaceToken="XXXXXX") {
    int templateIndex = templateStr.indexOf(replaceToken);

    if (templateIndex == -1) {
      return templateStr;
    }

    QString replacement = randomText(replaceToken.length());
    return templateStr.replace(templateIndex, replaceToken.length(), replacement);
  }

  /// converts a c++ std string into QByteArray, ensuring proper encoding
  static QByteArray stdStringToByteArray(std::string str) {
    return QByteArray(str.c_str(), str.size());
  }

  /**
   * @brief qstringToByteArray converts a QString into a QByteArray, ensuring proper encoding. Only
   * safe for ascii content.
   * @param q The string to convert
   * @return the QString as a QByteArray
   */
  static QByteArray qstringToByteArray(QString q) { return stdStringToByteArray(q.toStdString()); }

  /// writeFile write the provided content to the provided path.
  /// @throws a FileError if there are issues opening or writing to the file.
  static void writeFile(QString path, QString content) {
    writeFile(path, qstringToByteArray(content));
  }

  /// writeFile write the provided content to the provided path.
  /// @throws a FileError if there are issues opening or writing to the file.
  static void writeFile(QString path, QByteArray content) {
    QFile file(path);
    bool opened = file.open(QIODevice::WriteOnly);
    if (opened) {
      int bytesWritten = file.write(content);
      if (bytesWritten == -1) {
        throw FileError::mkError("Error writing file", path.toStdString(), file.error());
      }
      file.close();
    }
    if (file.error() != QFile::NoError) {
      throw FileError::mkError("Unable to write to file", path.toStdString(), file.error());
    }
  }

  /// readFile reads all of the data from the given path.
  /// @throws a FileError if any issues occur while writing the filethere are issues opening or
  /// reading the file.
  static QByteArray readFile(QString path) {
    auto result = readFileNoError(path);
    if (result.err != QFile::NoError) {
      throw FileError::mkError("Unable to read from file", path.toStdString(),
                               result.err);
    }
    return result.data;
  }

  /// mkdirs creates the necessary folders along a given path.
  /// equivalent to the unix mkdir -p command
  /// specify isFile = true if the path points to a file, rather than a directory
  static bool mkdirs(const QString& path, bool isFile = false) {
    auto adjustedPath = path;
    if (isFile) {
      adjustedPath = getDirname(path);
    }

    return QDir().mkpath(adjustedPath);
  }

  /// moveFile simply moves a file from one path to the next. Optionally, this method can create
  /// intermediary directories, as needed.
  /// equivalent to the unix mv command
  static bool moveFile(QString srcPath, QString dstPath, bool mkdirs = false) {
    if (mkdirs) {
      FileHelpers::mkdirs(dstPath, true);
    }

    QFile file(srcPath);
    return file.rename(dstPath);
  }

  /// copyFile simply copies a file from one path to the next. Optionally, this method can create
  /// intermediary directories, as needed.
  /// equivalent to the unix cp command
  static FileCopyResult copyFile(QString srcPath, QString dstPath, bool mkdirs = false) {
    FileCopyResult r;
    if (mkdirs) {
      FileHelpers::mkdirs(dstPath, true);
    }

    QFile file(srcPath);
    r.file = &file;
    r.success = file.copy(dstPath);

    return r;
  }


  /// getFilename is a small helper to convert a QFile into a filename (excluding the path)
  static QString getFilename(QFile f) { return QFileInfo(f).fileName(); }
  /// getFilename is a small helper to convert a filepath into a filename
  static QString getFilename(QString filepath) { return QFileInfo(filepath).fileName(); }
  /// getDirname is a small helper to convert a QFile pointing to a file into a path to that file's parent
  static QString getDirname(QFile f) { return QFileInfo(f).dir().path(); }
  /// getDirname is a small helper to convert a filepath to a file into a path to the file's parent
  static QString getDirname(QString filepath) {
    // maybe faster: filepath.left(filepath.lastIndexOf("/"));
    return QFileInfo(filepath).dir().path();
  }

  static FileReadResult readFileNoError(QString path) {
    QFile file(path);
    FileReadResult result;

    if(!file.open(QIODevice::ReadOnly)) {
      result.success = false;
      result.fileExists = file.exists();
    }
    else {
      result.fileExists = true;
      result.data = file.readAll();
      result.success = file.error() == QFile::NoError;
    }
    result.err = file.error();
    return result;
  }
};

#endif  // FILE_HELPERS_H
