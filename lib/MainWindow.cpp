#include "MainWindow.h"

#include "lzstring.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>

static bool loadJsonFromFile(const QString &filename, QJsonDocument &json) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  QByteArray line = file.readAll();
  file.close();
  QJsonParseError error;
  json = QJsonDocument::fromJson(line, &error);
  if (error.error)
    return false;

  return true;
}

static bool parseArray(const QString &filename, QJsonArray &arr) {
  QJsonDocument doc;
  if (!loadJsonFromFile(filename, doc) || !doc.isArray())
    return false;
  else
    arr = doc.array();
  return true;
}

bool MainWindow::loadContext() {
  bool success = true;
  errMsg = "[loadContext] ";
  QDir dataDir(wwwDir);

  if (!dataDir.cd("data")) {
    errMsg.append("Cannot find data dir");
    return false;
  }

  success &= parseArray(dataDir.filePath("Actors.json"), this->actors);
  success &= parseArray(dataDir.filePath("Armors.json"), this->armors);
  success &= parseArray(dataDir.filePath("Classes.json"), this->classes);
  success &= parseArray(dataDir.filePath("Items.json"), this->weapons);

  QJsonDocument doc;
  if (!loadJsonFromFile(dataDir.filePath("System.json"), doc) ||
      !doc.isObject())
    success = false;
  else
    this->system = doc.object();

  return success;
}

/// Tries to open and decode a save file. Will write filename into `fileName`,
/// and populate `decoded`.
/// Returns true if successful, otherwise false
bool MainWindow::openFile(QString filename) {
  errMsg = "[openFile] ";
  QFileInfo fileInfo(filename);
  while (fileInfo.isSymLink()) {
    fileInfo.setFile(fileInfo.symLinkTarget());
  }
  this->file.setFileName(fileInfo.absoluteFilePath());
  // If we cannot open file
  if (!this->file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    errMsg.append("Opening file failed: ");
    errMsg.append(filename);
    return false;
  }
  // Save the filename and decode into json
  QByteArray encoded = this->file.readLine();
  this->file.close();
  QString decoded = LZString::decompressFromBase64(encoded);

  if (decoded.isEmpty()) {
    errMsg.append("Failed to decode");
    return false;
  }

  QByteArray rawDecoded = decoded.toUtf8();

#ifndef NDEBUG
  QFile decodeDump("decoded.json");
  if (decodeDump.open(QIODevice::WriteOnly | QIODevice::Text)) {
    decodeDump.write(rawDecoded);
    decodeDump.close();
  } else
    qDebug() << "Error opening decoded.json\n";
#endif

  QJsonParseError error;
  auto saveDoc = QJsonDocument::fromJson(rawDecoded, &error);
  if (error.error) {
    errMsg.append(error.errorString());
    return false;
  }
  if (!saveDoc.isObject()) {
    errMsg.append("Invalid save format");
    return false;
  }

  this->save = saveDoc.object();

  QDir parentDir = fileInfo.dir();
  if (!parentDir.cdUp()) {
    return false;
  }

  if (wwwDir != parentDir) {
    wwwDir = parentDir;
    qDebug() << "[openFile] Updating wwwDir to " << wwwDir.absolutePath();
    if (!loadContext()) {
      errMsg.append("Failed to load some or all data files");
      return false;
    }
  }

  this->elements = std::make_unique<SaveElements>(this->save, this->actors);
  if (!elements->isValid()) {
    errMsg.append("Invalid save format");
    return false;
  }

  this->valid = true;

  return true;
}

MainWindow::MainWindow(int argc, char **argv) : toolbar(this), valid(false) {
  addToolBar(Qt::ToolBarArea::LeftToolBarArea, &toolbar);
  toolbar.setMovable(false);

  // If file is passed in on startup
  if (argc > 1) {
    if (!openFile(argv[1]))
      // TODO: Emit user-visible error
      qDebug() << errMsg;
  }
}
