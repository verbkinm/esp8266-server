#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>

#include "arduinopanel.h"

namespace Ui {
  class Widget;
}

class Widget : public QWidget
{
  Q_OBJECT

private:
  QTimer timer;
  int timeWork = 0;
  bool isOpen;
  QObject *parent;

  ArduinoPanel *arduino_panel;

  void setTimer(int d, int h, int m, int s, int ms);

public:
    Widget(QObject *prt);// *parent = 0);
    ~Widget();

    void enabledHost(bool b);
    void withArduino(bool b);
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
    void slotArduinoOpen();
    void slotArduinoClose();

signals:
    void signalPush(char);
    void signalReboot(char);
    void signalSendOther(char);
    void signalArduinoOpen();
};

#endif // WIDGET_H
