#include <QApplication>

#include "MainWindow.h"

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QCoreApplication::setApplicationName("WanderSaveEdit");
  QCoreApplication::setOrganizationName("WanderAway");

  MainWindow window(argc, argv);

  window.show();
  return app.exec();
}
