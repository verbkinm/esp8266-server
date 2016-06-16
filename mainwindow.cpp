#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QTime>
#include <QLabel>
#include <QThread>

#include "widget.h"

Widget* list[3];
int n = 3;

#define timeOut 5000

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->start_listening->setStyleSheet(QString::fromUtf8("background-color: rgb(63, 199, 72);"));

    serverListening = false;

    QVBoxLayout *pLayout = new QVBoxLayout;

    for(int i = 0; i < n; i++){
        list[i] = new Widget;
        pLayout->addWidget(list[i]);
        ui->groupBox->adjustSize();
    }
    ui->groupBox->setLayout(pLayout);
    ui->attention->setPixmap(QPixmap(":/attention-bw.png"));
    ui->attention->setPixmap(QPixmap(":/attention-bw.png"));

    this->setStatusBar(new QStatusBar);
    pStatusBarError = new QLabel("Ошибок нет =))");
    pStatusBarCountClients = new QLabel("Кол-во подключенных клиентов: 0");
    this->statusBar()->addWidget(pStatusBarCountClients);
    this->statusBar()->addWidget(pStatusBarError);

    connect(ui->ErrorList,        SIGNAL(triggered(bool)),SLOT(slotOpenErrorList()) );

    connect(ui->start_listening,  SIGNAL(clicked(bool)),  SLOT(slotStartServer())   );
    connect(ui->host,             SIGNAL(returnPressed()),SLOT(slotStartServer())   );
    connect(ui->port,             SIGNAL(returnPressed()),SLOT(slotStartServer())   );
    connect(this,                 SIGNAL(signalError(int)),SLOT(slotError(int))     );
    connect(&timerError,          SIGNAL(timeout()),      SLOT(slotTimeErrorOut())  );
}
void MainWindow::slotOpenErrorList()
{
    errorList = new ErrorList;
    errorList->show();
    ui->ErrorList->setEnabled(false);
    connect(errorList, SIGNAL(signalClose()), SLOT(slotCloseErrorList()) );
}
void MainWindow::slotCloseErrorList()
{
    disconnect(errorList, SIGNAL(signalClose()), this, SLOT(slotCloseErrorList()) );
    delete errorList;
    ui->ErrorList->setEnabled(true);
}
void MainWindow::slotTimeErrorOut()
{
    emit signalError(0);
}
void MainWindow::slotReboot(char byte)
{
    Widget* w = (Widget*)sender();
    for(int i = 0; i < n; i++)
        qDebug() << list[i];
    qDebug() << "write from " << w;
    map.value(w->getMacAddr())->write(&byte);
    qDebug() << "after write";
    map.value(w->getMacAddr())->close();
    qDebug() << "after close";
    disconnect(list[0],SIGNAL(signalReboot(char)), this, SLOT(slotReboot(char)) );
    ui->start_listening->setFocus();
}
void MainWindow::slotSendToClient(char byte)
{
    Widget* w = (Widget*)sender();
  //  qDebug() << WhoIsWidget(w->getMacAddr() );
    map.value(w->getMacAddr())->write(&byte);
}
void MainWindow::slotStartServer()
{
    if(!serverListening){
        m_ptcpServer = new QTcpServer(this);
        if(ui->port->text().isEmpty()){
            QMessageBox::critical(0,"Ошибка ввода","Ошибка ввода номера порта - пустое значение недопустимо!");
            ui->port->setFocus();
            return;
        }
        bool isOk = false;
        int value = ui->port->text().toInt(&isOk);
        if(isOk == false){
            QMessageBox::critical(0,"Ошибка ввода","Ошибка ввода номера порта - " +  ui->port->text() +", не является числовым значением!");
            ui->port->setFocus();
            return;
        }
        if(value < 1 || value > 65535){
            QMessageBox::critical(0,"Ошибка ввода","Ошибка ввода номера порта - " +  ui->port->text() + ", выходит за допустимые приделы(1-65535)!");
            ui->port->setFocus();
            return;
        }

        if (!m_ptcpServer->listen(QHostAddress(ui->host->text()), ui->port->text().toInt())) {
            QMessageBox::critical(0,"Server Error","Unable to start the server:"+ m_ptcpServer->errorString() );
            m_ptcpServer->close();
            return;
        }
        serverListening = true;
        ui->host->setEnabled(false);
        ui->port->setEnabled(false);
        ui->start_listening->setText("Стоп");
        ui->start_listening->setStyleSheet(QString::fromUtf8("background-color: rgb(240, 29, 29);"));
        connect(m_ptcpServer,         SIGNAL(newConnection()),SLOT(slotNewConnection()) );
        return;
    }
    if(serverListening){
        m_ptcpServer->close();
        delete m_ptcpServer;
        serverListening = false;
        ui->host->setEnabled(true);
        ui->port->setEnabled(true);
        ui->start_listening->setText("Пуска");
        ui->start_listening->setStyleSheet(QString::fromUtf8("background-color: rgb(63, 199, 72);"));
        for(int i = 0; i < n; i++){
            list[i]->enabledHost(false);
        }
        map.clear();
        return;
    }
}
/*virtual*/ void MainWindow::slotNewConnection()
{
  QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();
  qDebug() <<  "count connect" << ++countClient << "ip " << pClientSocket->peerAddress() << "port " << pClientSocket->peerPort() << "name " << pClientSocket->peerName();
  QString tmp = pStatusBarCountClients->text().mid(0,30);
  pStatusBarCountClients->setText(tmp + QString::number(countClient));

  connect(pClientSocket, SIGNAL(disconnected()),pClientSocket,  SLOT(deleteLater())     );
  connect(pClientSocket, SIGNAL(disconnected()),this,           SLOT(slotdisconnect())  );
  connect(pClientSocket, SIGNAL(readyRead()),   this,           SLOT(slotReadClient())  );

  qDebug() << m_ptcpServer->socketDescriptor();
}
int MainWindow::WhoIsWidget(QString mac)
{
    for(int i = 0; i < n; i++){
       if(list[i]->getMacAddr() == mac)
          return i;
    }
    return -1;
}
int MainWindow::WhoIsFree()
{
  for(int i = 0; i < n; i++){
      if(list[i]->getPing() == "обрыв")
          return i;
  }
  return -1;
}
void MainWindow::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QByteArray answer = pClientSocket->readAll();
    QString answerStr = answer.mid(4,17);
    if(answer.startsWith("MAC:")){
        qDebug() << map.keys();
        foreach (QString key, map.keys()) {
              if(answerStr == key){
                  emit signalError(1);
                  pClientSocket->close();
                  QThread::usleep(500);
                  qDebug() << "key " << key;
                  return;
              }
          }
        qDebug() << "!!!";
        map[answerStr] = pClientSocket;
        int free = WhoIsFree();
        if(free == -1){
            emit signalError(3);
            pClientSocket->close();
            map.remove(map.key((QTcpSocket*)sender()));
            return;
        }
        list[free]->enabledHost(true);
        list[free]->setMacAddr(answerStr);
        connect(list[free], SIGNAL(signalReboot(char)), this, SLOT(slotReboot(char))        );
        connect(list[free], SIGNAL(signalPush(char)),         SLOT(slotSendToClient(char))  );
        return;
    }
    if(answer.startsWith("low")){
        if( WhoIsWidget(map.key((QTcpSocket*)sender())) == -1 ){
            pClientSocket->close();
            map.remove(map.key((QTcpSocket*)sender()));
            return;
        }
        list[WhoIsWidget(map.key((QTcpSocket*)sender()))]->setVal(0);
        return;
    }
    if(answer.startsWith("high")){
        if( WhoIsWidget(map.key((QTcpSocket*)sender())) == -1 ){
            pClientSocket->close();
            map.remove(map.key((QTcpSocket*)sender()));
            return;
        }
        list[WhoIsWidget(map.key((QTcpSocket*)sender()))]->setVal(1);
        return;
    }
    if(answer.startsWith("ping")){
        if( WhoIsWidget(map.key((QTcpSocket*)sender())) == -1 ){
            pClientSocket->close();
            map.remove(map.key((QTcpSocket*)sender()));
            return;
        }
        timer[WhoIsWidget(map.key((QTcpSocket*)sender()))].start(timeOut);
        connect(&timer[WhoIsWidget(map.key((QTcpSocket*)sender()))],SIGNAL(timeout()), SLOT(slotTimeOut()) );
        return;
    }
    emit signalError(2);
    pClientSocket->close();// disconnected();
}
void MainWindow::slotdisconnect()
{
    if(map.size() > 0){
        qDebug() << "test1";
        qDebug() << WhoIsWidget(map.key((QTcpSocket*)sender()));
        int i = WhoIsWidget(map.key((QTcpSocket*)sender()));
        if(i != -1){
            list[i]->enabledHost(false);
            qDebug() << "test2";
            timer[i].stop();
            map.remove(map.key((QTcpSocket*)sender()));
        }
        qDebug() << "test3";
        qDebug() <<  "count disconnected" << --countClient;
        QString tmp = pStatusBarCountClients->text().mid(0,30);
        pStatusBarCountClients->setText(tmp + QString::number(countClient));
    }
}
int MainWindow::WhoseIsTimer(QTimer *t)
{
  for(int i = 0; i < n; i++){
      if(t == &timer[i])
          return i;
  }
  return -1;
}
void MainWindow::slotTimeOut()
{
    qobject_cast<QTimer*>(sender())->stop();
    qDebug() << "timer stop";
    map.value(list[WhoseIsTimer(qobject_cast<QTimer*>(sender()))]->getMacAddr())->close();
    qDebug() << "timer stop";
    disconnect(qobject_cast<QTimer*>(sender()),SIGNAL(timeout()), this, SLOT(slotTimeOut()) );
}
void MainWindow::slotError(int err)
{
    qDebug() << "slotError " << err;
    ui->attention->setPixmap(QPixmap(":/attention.png"));
    timerError.start(10000);
    switch (err) {
      case 0:
        ui->attention->setPixmap(QPixmap(":/attention-bw.png"));
        pStatusBarError->setText("Ошибок нет =))");
        timerError.stop();
        break;
      case 1:
        pStatusBarError->setText("Ошибка: " + QString::number(err));
        break;
      case 2:
        pStatusBarError->setText("Ошибка: " + QString::number(err));
        break;
      case 3:
        pStatusBarError->setText("Ошибка: " + QString::number(err));
        break;
      default:
        break;
      }
}
/*virtual*/ void MainWindow::closeEvent(QCloseEvent *)
{
  foreach (QObject* var,  this->children() ) {
//      var.close();
      qDebug() << var->objectName();
    }
  exit(0);
}

MainWindow::~MainWindow()
{
  delete ui;
}
