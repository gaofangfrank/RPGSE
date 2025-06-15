#include "MainWindow.h"
#include "lzstring.h"
#include <QDebug>
#include <QFile>

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

bool MainWindow::loadContext() {
  bool success = true;
  errMsg = "[loadContext] ";
  QDir dataDir(wwwDir);

  if (!dataDir.cd("data")) {
    errMsg.append("Cannot find data dir");
    return false;
  }
  if (!loadJsonFromFile(dataDir.filePath("Actors.json"), this->actors))
    success = false;
  if (!loadJsonFromFile(dataDir.filePath("Armors.json"), this->armors))
    success = false;
  if (!loadJsonFromFile(dataDir.filePath("Classes.json"), this->classes))
    success = false;
  if (!loadJsonFromFile(dataDir.filePath("Items.json"), this->items))
    success = false;
  if (!loadJsonFromFile(dataDir.filePath("Weapons.json"), this->weapons))
    success = false;
  if (!loadJsonFromFile(dataDir.filePath("System.json"), this->system))
    success = false;
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
  file.setFileName(fileInfo.absoluteFilePath());
  // If we cannot open file
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    errMsg.append("Opening file failed: ");
    errMsg.append(filename);
    return false;
  }
  // Save the filename and decode into json
  QByteArray encoded = file.readLine();
  file.close();
  QString decoded = LZString::decompressFromBase64(encoded);

  if (decoded.isEmpty()) {
    errMsg.append("Failed to decode");
    return false;
  }

  QByteArray rawDecoded = decoded.toUtf8();
  QJsonParseError error;
  this->save = QJsonDocument::fromJson(rawDecoded, &error);
  if (error.error) {
    errMsg.append(error.errorString());
    return false;
  }

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

  return true;
}

MainWindow::MainWindow(int argc, char **argv) : toolbar(this) {
  addToolBar(Qt::ToolBarArea::LeftToolBarArea, &toolbar);
  toolbar.setMovable(false);

  // If file is passed in on startup
  if (argc > 1) {
    if (!openFile(argv[1]))
      // TODO: Emit user-visible error
      qDebug() << errMsg;
  }
}
