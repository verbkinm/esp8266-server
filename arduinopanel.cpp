#include "arduinopanel.h"
#include "ui_arduinopanel.h"
//#include "mainwindow.h"
#include <QDebug>

ArduinoPanel::ArduinoPanel(QObject *prt) :
  QWidget(),
  ui(new Ui::ArduinoPanel)
{
//  parent = prt;
//  MainWindow *main_window = qobject_cast<MainWindow*>(parent);

//  qDebug() << "main_window n = " << main_window->n;
//  main_window->slotSendToClient('5');

  ui->setupUi(this);
}
/*virtual*/ void ArduinoPanel::closeEvent(QCloseEvent *)
{
    emit signalClose();
}
ArduinoPanel::~ArduinoPanel()
{
  delete ui;
}
