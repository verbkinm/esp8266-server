#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>

namespace Ui {
  class Widget;
}

class Widget : public QWidget
{
  Q_OBJECT

private:
  QTimer timer;
  int timeWork = 0;

  void setTimer(int d, int h, int m, int s, int ms);

public:
    Widget(QWidget *parent = 0);
    ~Widget();

    void enabledHost(bool b);
    void setMacAddr(QString mac);
    void setVal(int bit);
    QString getWorkingTime();
    const QString getMacAddr();
    const QString getPing();

    Ui::Widget *ui;

private slots:
    void slot_on_off();
    void slotReboot();
    void slotTimerOut();

signals:
    void signalPush(char);
    void signalReboot(char);
};

#endif // WIDGET_H
