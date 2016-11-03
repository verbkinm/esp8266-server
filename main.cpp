#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w;
  w.show();

//  a.setStyleSheet("QPushButton{border: 1px solid black; border-radius:25;}");
  return a.exec();
}
