#include <QDir>
#include <QJsonDocument>
#include <QMainWindow>
#include <QToolBar>

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(int argc, char **argv);

private:
  bool openFile(QString filename);
  bool loadContext();

  QToolBar toolbar;

  // Json loaded from www/data
  QJsonDocument actors;
  QJsonDocument armors;
  QJsonDocument classes;
  QJsonDocument items;
  QJsonDocument weapons;
  QJsonDocument system;

  QJsonDocument save;
  QDir wwwDir;
  QFile file;
  QString errMsg;
};
