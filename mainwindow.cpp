#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QTime>
#include <QLabel>

#include "widget.h"
int n = 3;
Widget* list[3];

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
        connect(list[i], SIGNAL(signalPush(char)),  SLOT(slotSendToClient(char))  );
        connect(list[i], SIGNAL(signalReboot(char)),SLOT(slotReboot(char))        );
    }
    ui->groupBox->setLayout(pLayout);

    connect(ui->start_listening,  SIGNAL(clicked(bool)),  SLOT(slotStartServer())   );
    connect(ui->host,             SIGNAL(returnPressed()),SLOT(slotStartServer())   );
}
void MainWindow::slotReboot(char byte)
{
  Widget* w = (Widget*)sender();
  qDebug() << "write";
  map.value(w->getMacAddr())->write(&byte);
  map.value(w->getMacAddr())->close();
  qDebug() << map.keys();
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
        if (!m_ptcpServer->listen(QHostAddress(ui->host->text()), ui->port->text().toInt())) {
            QMessageBox::critical(0,"Server Error","Unable to start the server:"+ m_ptcpServer->errorString() );
            m_ptcpServer->close();
            return;
        }
        serverListening = true;
        ui->host->setEnabled(false);
        ui->port->setEnabled(false);
        ui->start_listening->setText("Останов сервера");
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
        ui->start_listening->setText("Пуск сервера");
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

  connect(pClientSocket, SIGNAL(disconnected()),pClientSocket,  SLOT(deleteLater())     );
  connect(pClientSocket, SIGNAL(disconnected()),this,           SLOT(slotdisconnect())  );
  connect(pClientSocket, SIGNAL(readyRead()),   this,           SLOT(slotReadClient())  );
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
    if(answer.startsWith("MAC:")){
        map[answer.mid(4,20)] = pClientSocket;
        int free = WhoIsFree();
        if(free == -1){
            pClientSocket->close();
            map.remove(map.key((QTcpSocket*)sender()));
            return;
        }
        list[free]->enabledHost(true);
        list[free]->setMacAddr(answer.mid(4,20) );
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
    pClientSocket->close();// disconnected();
}
void MainWindow::sendToClient(QTcpSocket* pSocket, const char str)
{
//    pSocket->write(&str);
}
void MainWindow::slotOnOff()
{
//  if(ui->on_off->text() == "1")
//  {
//      ui->on_off->setText("0");
//      sendToClient(map.value(ui->macaddr->text()),'0');
//      return;
//  }
//  if(ui->on_off->text() == "0")
//  {
//      ui->on_off->setText("1");
//      sendToClient(map.value(ui->macaddr->text()),'1');
//      return;
//  }
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
MainWindow::~MainWindow()
{
  delete ui;
}
