#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QMap>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    Ui::MainWindow *ui;
    QTcpServer* m_ptcpServer;
    QTimer timer[3];
    QMap<QString, QTcpSocket*> map;
    bool serverListening;



    void sendToClient(QTcpSocket* pSocket, const char str);
    int WhoIsWidget(QString mac);
    int WhoseIsTimer(QTimer* t);
    int WhoIsFree();
public slots:
            void slotStartServer  ();
    virtual void slotNewConnection();
            void slotReadClient   ();
            void slotOnOff        ();
            void slotdisconnect   ();
            void slotTimeOut      ();
            void slotReboot       (char byte);
            void slotSendToClient (char byte);
};

#endif // MAINWINDOW_H
