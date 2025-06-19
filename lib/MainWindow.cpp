#include "MainWindow.h"

#include "lzstring.h"

#include <QDebug>
#include <QFile>

template <typename JSONT,
          typename = std::enable_if_t<std::is_same_v<JSONT, json::Object> ||
                                      std::is_same_v<JSONT, json::Array>>>
static bool loadJsonFromFile(const QString &filename, JSONT &json,
                             QString &errMsg) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  QByteArray data = file.readAll();
  file.close();
  llvm::StringRef jsonStr(data.constData());
  auto maybeObj = json::parse(jsonStr);
  if (!maybeObj) {
    if (handleErrors(maybeObj.takeError(), [&](const json::ParseError &pe) {
          errMsg.append(pe.message().c_str());
        }).success())
      qDebug() << "handleError on parse somehow failed";
    return false;
  }

  if constexpr (std::is_same<JSONT, json::Object>()) {
    auto *obj = maybeObj->getAsObject();
    json = std::move(*obj);
  } else if constexpr (std::is_same<JSONT, json::Array>()) {
    auto *arr = maybeObj->getAsArray();
    json = std::move(*arr);
  }

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

  success &=
      loadJsonFromFile(dataDir.filePath("Actors.json"), this->actors, errMsg);
  success &=
      loadJsonFromFile(dataDir.filePath("Armors.json"), this->armors, errMsg);
  success &=
      loadJsonFromFile(dataDir.filePath("Classes.json"), this->classes, errMsg);
  success &=
      loadJsonFromFile(dataDir.filePath("Items.json"), this->weapons, errMsg);

  if (!loadJsonFromFile(dataDir.filePath("System.json"), this->system, errMsg))
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

  auto maybeSave = json::parse(rawDecoded.data());
  if (!maybeSave) {
    if (handleErrors(maybeSave.takeError(), [&](const json::ParseError &pe) {
          errMsg.append(pe.message().c_str());
        }).success())
      qDebug() << "handleError on parse somehow failed";
    return false;
  }

  auto *saveObj = maybeSave->getAsObject();
  if (!saveObj) {
    errMsg.append("Expecting save file to be an json object");
    return false;
  }

  this->save = std::move(*saveObj);

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

  this->elements =
      std::make_unique<SaveElements>(this->save, this->actors, this->system);
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
