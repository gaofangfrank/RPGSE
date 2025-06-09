#include "MainWindow.h"
#include "lzstring.h"
#include <QDebug>
#include <QFile>

/// Tries to open and decode a save file. Will write filename into `fileName`,
/// and populate `decoded`.
bool MainWindow::openFile(char *filename) {
  QFile file(filename);
  // If we cannot open file
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug("Opening file failed: %s", filename);
    // TODO: Emit error
    return false;
  }

  // Save the filename and decode into json
  this->fileName = filename;
  QByteArray encoded = file.readLine();
  QString decoded = LZString::decompressFromBase64(encoded);
  file.close();

  if (decoded.isEmpty()) {
    qDebug("Failed to decode");
    // TODO: Emit user-visible error
    return false;
  }

  QByteArray rawDecoded = decoded.toUtf8();
  QJsonParseError error;
  this->decoded.fromJson(rawDecoded, &error);
  if (error.error) {
    qDebug() << "[JSON] " << error.errorString();
    // TODO: Emit user-visible error
    return false;
  }

  return true;
}

MainWindow::MainWindow(int argc, char **argv) : toolbar(this) {
  addToolBar(Qt::ToolBarArea::LeftToolBarArea, &toolbar);
  toolbar.setMovable(false);

  // If file is passed in on startup
  if (argc > 1)
    openFile(argv[1]);
}
