#include "logfileread.h"
#include "ui_logfileread.h"
#include <QScrollBar>

LogFileRead::LogFileRead(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::LogFileRead)
{
  ui->setupUi(this);
  connect(ui->pushButton, SIGNAL(clicked(bool)), SIGNAL(signalClearLogFile()) );
}
/*virtual*/ void LogFileRead::closeEvent(QCloseEvent *)
{
    emit signalClose();
}
void LogFileRead::addText(QString text)
{
  ui->textEdit->append(text);
  this->scrollToEnd();
}
void LogFileRead::scrollToEnd()
{
  QTextCursor c = ui->textEdit->textCursor();
  c.movePosition(QTextCursor::End);
  ui->textEdit->setTextCursor(c);
}
void LogFileRead::clear()
{
  ui->textEdit->clear();
}
LogFileRead::~LogFileRead()
{
  delete ui;
}
