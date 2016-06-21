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
    QTimer                      timer[3];
    QMap<QString, QTcpSocket*>  map;
    bool                        serverListening, led;
    int                         countClient = 0;
    QTimer                      timerError, timerBlink;
    QLabel                      *pStatusBarError, *pStatusBarCountClients;
    QMediaPlayer*               player;

    ErrorList *errorList;
    LogFileRead *logFileRead;
    Log loging;
//    QFile file;


    int WhoIsWidget(QString mac);
    int WhoseIsTimer(QTimer* t);
    int WhoIsFree();
    void appendLogWindow();

    virtual void closeEvent(QCloseEvent*);

private slots:
            void slotStartServer  ();
    virtual void slotNewConnection();
            void slotReadClient   ();
            void slotdisconnect   ();
            void slotTimeOut      ();
            void slotTimeErrorOut ();
            void slotTimerBlink   ();
            void slotReboot       (char byte);
            void slotSendToClient (char byte);
            void slotError        (int err);
            void slotSocketError  (QAbstractSocket::SocketError err);

            void slotOpenErrorList();
            void slotCloseErrorList();
            void slotOpenLog      ();
            void slotCloseLog     ();
            void slotLogFileChanged();

signals:
            void signalError      (int);
};

#endif // MAINWINDOW_H
