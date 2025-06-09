#include <QJsonDocument>
#include <QMainWindow>
#include <QToolBar>

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(int argc, char **argv);

private:
  bool openFile(char *filename);
  QToolBar toolbar;
  QJsonDocument decoded;
  QString fileName;
};
