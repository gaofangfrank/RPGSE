#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "llvm/Support/JSON.h"
#include <QDir>
#include <QMainWindow>
#include <QToolBar>

#include "SaveElements.h"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(int argc, char **argv);

private:
  bool openFile(QString filename);
  bool loadContext();

  QToolBar toolbar;

  // Json loaded from www/data
  json::Array actors;
  json::Array armors;
  json::Array classes;
  json::Array items;
  json::Array weapons;
  json::Object system;

  json::Object save;
  std::unique_ptr<SaveElements> elements;

  QDir wwwDir;
  QFile file;
  QString errMsg;

  bool valid;
};

#endif
