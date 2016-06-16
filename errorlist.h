#ifndef ERRORLIST_H
#define ERRORLIST_H

#include <QWidget>
#include <QMainWindow>

namespace Ui {
  class ErrorList;
}

class ErrorList : public QWidget
{
  Q_OBJECT

public:  
    ErrorList(QMainWindow *parent = 0);
    ~ErrorList();

    virtual void closeEvent(QCloseEvent*);

signals:
    void signalClose();

private:
    Ui::ErrorList *ui;
};

#endif // ERRORLIST_H
