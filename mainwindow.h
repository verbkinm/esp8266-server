#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QMap>
#include <QLabel>
#include <QMediaPlayer>
#include <QFile>

#include "errorlist.h"
#include "log.h"
#include "logfileread.h"
#include "widget.h"

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
    Ui::MainWindow              *ui;
    QTcpServer*                 m_ptcpServer;
    QTimer                      timer[3];   //timer for ping
    QMap<QString, QTcpSocket*>  map;
    bool                        serverListening, led;
    int                         countClient = 0;
    QTimer                      timerError, timerBlink;
    QLabel                      *pStatusBarError, *pStatusBarCountClients;
    QMediaPlayer*               player;

    ErrorList *errorList;
    LogFileRead *logFileRead;
    Log loging;

    Widget* list[3];
    int n = 3;      // count of widgets for client

    int WhoIsWidget       (QString mac);
    int WhoseIsTimer      (QTimer* t);
    int WhoIsFree         ();
    void appendLogWindow  ();
    void logging          (QString text);

    virtual void closeEvent(QCloseEvent*);

private slots:
            void slotStartServer  ();
    virtual void slotNewConnection();
            void slotReadClient   ();
            void slotdisconnect   ();
            void slotTimeOut      ();
            void slotTimeErrorOut ();
            void slotTimerBlinkOut();
            void slotReboot       (char byte);
            void slotSendToClient (char byte);
            void slotError        (int err);
            void slotSocketError  (QAbstractSocket::SocketError err);

            void slotOpenErrorList();
            void slotCloseErrorList();
            void slotOpenLog      ();
            void slotCloseLog     ();

signals:
            void signalError      (int);
};

#endif // MAINWINDOW_H
