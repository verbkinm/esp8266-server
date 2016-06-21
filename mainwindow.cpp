#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QTime>
#include <QLabel>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QTextEdit>

#include "widget.h"

Widget* list[3];
int n = 3;


#define timeOut 5000

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
    loging.append("==============================");
    loging.append("Application start");

    ui->setupUi(this);
    ui->start_listening->setStyleSheet(QString::fromUtf8("background-color: rgb(63, 199, 72);"));

    serverListening = false;
    led             = false;

    QVBoxLayout *pLayout = new QVBoxLayout;

    for(int i = 0; i < n; i++){
        list[i] = new Widget;
        pLayout->addWidget(list[i]);
        ui->groupBox->adjustSize();
    }
    ui->groupBox->setLayout(pLayout);
    ui->attention->setPixmap(QPixmap(":/attention-bw.png"));

    this->setStatusBar(new QStatusBar);
    pStatusBarError = new QLabel("Ошибок нет =))");
    pStatusBarCountClients = new QLabel("Кол-во подключенных клиентов: 0");
    this->statusBar()->addWidget(pStatusBarCountClients);
    this->statusBar()->addWidget(pStatusBarError);

    QString fileName = QDir::currentPath() + "\\tmp.mp3";
    QFile fileQrc(":/attention.mp3");
    fileQrc.open(QIODevice::ReadOnly);
    QByteArray ba = fileQrc.readAll();
    QFile fileTmp(fileName);
    fileTmp.open(QIODevice::WriteOnly);
    fileTmp.write(ba);
    fileTmp.close();

    player = new QMediaPlayer;
    player->setMedia(QUrl::fromLocalFile(fileName));

    connect(ui->ErrorList,        SIGNAL(triggered(bool)),SLOT(slotOpenErrorList()) );
    connect(ui->log,              SIGNAL(triggered(bool)),SLOT(slotOpenLog())       );

    connect(ui->start_listening,  SIGNAL(clicked(bool)),  SLOT(slotStartServer())   );
    connect(ui->host,             SIGNAL(returnPressed()),SLOT(slotStartServer())   );
    connect(ui->port,             SIGNAL(returnPressed()),SLOT(slotStartServer())   );
    connect(this,                 SIGNAL(signalError(int)),SLOT(slotError(int))     );
    connect(&timerError,          SIGNAL(timeout()),      SLOT(slotTimeErrorOut())  );
    connect(&timerBlink,          SIGNAL(timeout()),      SLOT(slotTimerBlink())    );
}
void MainWindow::slotOpenErrorList()
{
    errorList = new ErrorList;
    errorList->show();
    ui->ErrorList->setEnabled(false);
    connect(errorList, SIGNAL(signalClose()), SLOT(slotCloseErrorList()) );
}
void MainWindow::slotOpenLog()
{
  logFileRead = new LogFileRead;
  QFile file(QString(QDir::currentPath().replace("/","\\") + "\\esp-01.log") );
  file.open(QIODevice::ReadOnly);
  QString text = file.readAll();
  logFileRead->addText(text);
  logFileRead->show();
  ui->log->setEnabled(false);
//  connect(&file, SIGNAL( bytesWritten(qint64)), SLOT(slotLogFileChanged())  );
  connect(logFileRead, SIGNAL(signalClose()), SLOT(slotCloseLog()) );
}
void MainWindow::slotLogFileChanged()
{
  qDebug() << "file changed";
//  QFile file(QString(QDir::currentPath().replace("/","\\") + "\\esp-01.log") );
//  file.open(QIODevice::ReadOnly);
//  QString text = file.readAll();
//  logFileRead->clear();
//  logFileRead->addText(text);
}
void MainWindow::slotCloseErrorList()
{
//    disconnect(&file, SIGNAL(bytesWritten(qint64)), this, SLOT(slotLogFileChenged())  );
    disconnect(errorList, SIGNAL(signalClose()), this, SLOT(slotCloseErrorList()) );
    delete errorList;
    ui->ErrorList->setEnabled(true);
}
void MainWindow::slotCloseLog()
{
  disconnect(logFileRead, SIGNAL(signalClose()), this, SLOT(slotCloseLog()) );
  delete logFileRead;
  ui->log->setEnabled(true);
}

void MainWindow::slotTimeErrorOut()
{
    emit signalError(0);
}
void MainWindow::slotTimerBlink()
{
    if(led){
        ui->attention->setPixmap(QPixmap(":/attention-bw.png"));
        led = false;
        return;
    }
    if(!led){
        ui->attention->setPixmap(QPixmap(":/attention.png"));
        led = true;
        return;
    }
}
void MainWindow::slotReboot(char byte)
{
    Widget* w = (Widget*)sender();
    QTcpSocket* pClientSocket = map.value(w->getMacAddr());
    for(int i = 0; i < n; i++)
        qDebug() << list[i];
    qDebug() << "write from " << w;
    loging.append("rebooting client: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) );
    appendLogWindow();
    pClientSocket->write(&byte);
    qDebug() << "after write";
    pClientSocket->close();
    qDebug() << "after close";
    int i = WhoIsWidget(w->getMacAddr());
    disconnect(list[i], SIGNAL(signalReboot(char)), this, SLOT(slotReboot(char))        );
    disconnect(list[i], SIGNAL(signalPush(char)),   this, SLOT(slotSendToClient(char))  );

    ui->start_listening->setFocus();
}
void MainWindow::slotSendToClient(char byte)
{
    Widget* w = (Widget*)sender();
    QTcpSocket* pClientSocket = map.value(w->getMacAddr());
    pClientSocket->write(&byte);
    loging.append("send " + QString(byte) + " to client: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) );
    appendLogWindow();
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
        connect(m_ptcpServer, SIGNAL(newConnection()), SLOT(slotNewConnection()) );
        loging.append("Server start");
        appendLogWindow();
        return;
    }
    if(serverListening){
        m_ptcpServer->close();
        delete m_ptcpServer;
        serverListening = false;
        ui->host->setEnabled(true);
        ui->port->setEnabled(true);
        ui->start_listening->setText("Пуск");
        ui->start_listening->setStyleSheet(QString::fromUtf8("background-color: rgb(63, 199, 72);"));
        for(int i = 0; i < n; i++){
            list[i]->enabledHost(false);
        }
        map.clear();
        loging.append("Server stop");
        appendLogWindow();
        return;
    }
}
/*virtual*/ void MainWindow::slotNewConnection()
{
  QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();
  loging.append("connecting peer: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) );
  appendLogWindow();
  QString tmp = pStatusBarCountClients->text().mid(0,30);
  pStatusBarCountClients->setText(tmp + QString::number(++countClient));

  connect(pClientSocket, SIGNAL(disconnected()),pClientSocket,  SLOT(deleteLater())     );
  connect(pClientSocket, SIGNAL(disconnected()),this,           SLOT(slotdisconnect())  );
  connect(pClientSocket, SIGNAL(readyRead()),   this,           SLOT(slotReadClient())  );
  connect(pClientSocket, SIGNAL(error(QAbstractSocket::SocketError)),SLOT(slotSocketError(QAbstractSocket::SocketError)) );

  qDebug() << m_ptcpServer->socketDescriptor();
}
void MainWindow::slotSocketError(QAbstractSocket::SocketError err)
{
  qDebug() << "Error socket " << err;
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
void MainWindow::appendLogWindow()
{
  if(!ui->log->isEnabled()){
    QFile file(QString(QDir::currentPath().replace("/","\\") + "\\esp-01.log") );
    file.open(QIODevice::ReadOnly);
    logFileRead->addText(file.readAll());
  }
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
                  loging.append("ERROR 1(conflict the key-addresses) Connecting client: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()));
                  appendLogWindow();
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
            loging.append("ERROR 3(There is not enough free space on the control panel MO-1) Connecting client: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()));
            appendLogWindow();
            emit signalError(3);
            pClientSocket->close();
            map.remove(map.key((QTcpSocket*)sender()));
            return;
        }
        list[free]->enabledHost(true);
        list[free]->setMacAddr(answerStr);
        connect(list[free], SIGNAL(signalReboot(char)), SLOT(slotReboot(char))        );
        connect(list[free], SIGNAL(signalPush(char)),   SLOT(slotSendToClient(char))  );
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
    loging.append("ERROR 2(Error client request) Connecting client: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()));
    appendLogWindow();
    emit signalError(2);
    pClientSocket->close();// disconnected();
}
void MainWindow:: slotdisconnect()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    if(map.size() > 0){
        qDebug() << "test1";
        qDebug() << WhoIsWidget(map.key((QTcpSocket*)sender()));
        int i = WhoIsWidget(map.key((QTcpSocket*)sender()));
        if(i != -1){
            list[i]->enabledHost(false);
            disconnect(list[i], SIGNAL(signalReboot(char)), this, SLOT(slotReboot(char))        );
            disconnect(list[i], SIGNAL(signalPush(char)),   this, SLOT(slotSendToClient(char))  );
            qDebug() << "test2";
            timer[i].stop();
            map.remove(map.key((QTcpSocket*)sender()));
        }
    }
    QString tmp = pStatusBarCountClients->text().mid(0,30);
    pStatusBarCountClients->setText(tmp + QString::number(--countClient));
    loging.append("disconnecting peer: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) );
    appendLogWindow();
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
    led = true;
    timerError.start(10000);
    timerBlink.start(300);
    player->stop();
    if(err != 0)
        player->play();
    qDebug() << "start";
    switch (err) {
      case 0:
        player->stop();
        qDebug() << "stop";
        ui->attention->setPixmap(QPixmap(":/attention-bw.png"));
        led = false;
        pStatusBarError->setText("Ошибок нет =))");
        timerError.stop();
        timerBlink.stop();
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
    QString fileName = QDir::currentPath() + "\\tmp.mp3";
    QFile file(fileName);
    file.remove();

    foreach (QTcpSocket* client, map.values()) {
        client->close();
    }
    loging.append("Server stop");
    loging.append("Application stop");
    loging.append("==============================");
    appendLogWindow();
    exit(0);
}

MainWindow::~MainWindow()
{
  delete ui;
}
