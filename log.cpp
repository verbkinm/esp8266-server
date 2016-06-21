#include "log.h"
#include <QDateTime>
#include <QDir>
#include <QTextStream>

Log::Log()
{

}
void Log::append(QString str)
{
  QString logCurrentPath = QDir::currentPath().replace("/","\\") + "\\esp-01.log";
  this->setFileName(logCurrentPath);
  this->open(QIODevice::WriteOnly | QIODevice::Append);
  QTextStream ts(this);
  ts << QDateTime::currentDateTime().toString("dd.MM.yy hh:mm:ss") << " - " << str << endl;
  this->close();
}
