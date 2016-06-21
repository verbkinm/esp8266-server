#ifndef LOG_H
#define LOG_H

#include <QFile>


class Log : public QFile
{
public:
  Log();

  void append(QString str);
};

#endif // LOG_H
