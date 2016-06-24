#include "errorlist.h"
#include "ui_errorlist.h"

#include <QDebug>

ErrorList::ErrorList(QMainWindow *parent) :
  QWidget(parent),
  ui(new Ui::ErrorList)
{
  ui->setupUi(this);
}
/*virtual*/ void ErrorList::closeEvent(QCloseEvent *)
{
    emit signalClose();
}
ErrorList::~ErrorList()
{
  delete ui;
}
