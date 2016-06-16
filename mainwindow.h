#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QMap>
#include <QLabel>

#include "errorlist.h"

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
    bool                        serverListening;
    int                         countClient = 0;
    QTimer                      timerError;
    QLabel                      *pStatusBarError, *pStatusBarCountClients;

    ErrorList *errorList;

    int WhoIsWidget(QString mac);
    int WhoseIsTimer(QTimer* t);
    int WhoIsFree();

    virtual void closeEvent(QCloseEvent*);

public slots:
            void slotStartServer  ();
    virtual void slotNewConnection();
            void slotReadClient   ();
            void slotdisconnect   ();
            void slotTimeOut      ();
            void slotTimeErrorOut ();
            void slotReboot       (char byte);
            void slotSendToClient (char byte);
            void slotError        (int err);

            void slotOpenErrorList();
            void slotCloseErrorList();
signals:
            void signalError       (int);
};

#endif // MAINWINDOW_H
