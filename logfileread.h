#ifndef LOGFILEREAD_H
#define LOGFILEREAD_H

#include <QWidget>

namespace Ui {
  class LogFileRead;
}

class LogFileRead : public QWidget
{
  Q_OBJECT

public:
  LogFileRead(QWidget *parent = 0);
  ~LogFileRead();

          void addText(QString text);
          void scrollToEnd();
          void clear();
  virtual void closeEvent(QCloseEvent*);

signals:
  void signalClose();
  void signalClearLogFile();

private:
  Ui::LogFileRead *ui;
};

#endif // LOGFILEREAD_H
