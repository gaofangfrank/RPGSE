#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
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
  QJsonArray actors;
  QJsonArray armors;
  QJsonArray classes;
  QJsonArray items;
  QJsonArray weapons;
  QJsonObject system;

  QJsonObject save;
  std::unique_ptr<SaveElements> elements;

  QDir wwwDir;
  QFile file;
  QString errMsg;

  bool valid;
};

#endif
