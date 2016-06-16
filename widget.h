#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QMap>

namespace Ui {
  class Widget;
}

class Widget : public QWidget
{
  Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

    void enabledHost(bool b);
    void setMacAddr(QString mac);
    void setVal(int bit);
    const QString getMacAddr();
    const QString getPing();

    Ui::Widget *ui;

private slots:
    void slot_on_off();
    void slotReboot();
signals:
    void signalPush(char);
    void signalReboot(char);

};

#endif // WIDGET_H
