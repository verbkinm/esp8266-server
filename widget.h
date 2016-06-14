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
  Widget(int nPort, QWidget *parent = 0);
  ~Widget();

private:
  QTcpServer* m_ptcpServer;
  Ui::Widget *ui;
  QTimer timer;
  QMap<QString, QTcpSocket*> map;

private:
    void sendToClient(QTcpSocket* pSocket, const char str);

public slots:
    virtual void slotNewConnection();
            void slotReadClient   ();
            void slotOnOff        ();
            void slotdisconnect   ();
            void slotTimeOut      ();
            void slotReboot       ();
};

#endif // WIDGET_H
