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

#include <QDebug>

#define TIME_OUT 7000
#define TIME_OUT_PING 5000
#define MAX_CLIENTS 3

MainWindow::MainWindow(QObject *parent) :
  QMainWindow(),
  ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->start_listening->setStyleSheet(QString::fromUtf8("background-color: rgb(63, 199, 72);"));

    serverListening = false;
    led             = false;

    QVBoxLayout *pLayout = new QVBoxLayout;

    for(int i = 0; i < n; i++){
        list[i] = new Widget(this);
        pLayout->addWidget(list[i]);
//        connect(list[i], SIGNAL(signalArduinoOpen()), SLOT() );
    }
    ui->groupBox->adjustSize();

    ui->groupBox->setLayout(pLayout);
    ui->attention->setPixmap(QPixmap(":/attention-bw.png"));

    this->setStatusBar(new QStatusBar);
    pStatusBarError = new QLabel("Ошибок нет =))");
    pStatusBarCountClients = new QLabel("Кол-во подключенных клиентов: 0");
    this->statusBar()->addWidget(pStatusBarCountClients);
    this->statusBar()->addWidget(pStatusBarError);

    player = new QMediaPlayer;
    player->setMedia(QUrl("qrc:/attention.mp3"));

    connect(ui->ErrorList,        SIGNAL(triggered(bool)),SLOT(slotOpenErrorList()) );
    connect(ui->log,              SIGNAL(triggered(bool)),SLOT(slotOpenLog())       );

    connect(ui->start_listening,  SIGNAL(clicked(bool)),  SLOT(slotStartServer())   );
    connect(ui->host,             SIGNAL(returnPressed()),SLOT(slotStartServer())   );
    connect(ui->port,             SIGNAL(returnPressed()),SLOT(slotStartServer())   );
    connect(this,                 SIGNAL(signalError(int)),SLOT(slotError(int))     );
    connect(&timerError,          SIGNAL(timeout()),      SLOT(slotTimeErrorOut())  );
    connect(&timerBlink,          SIGNAL(timeout()),      SLOT(slotTimerBlinkOut()) );

    logging("==============================");
    logging("Application start");

    QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    QRegExp ipRegex ("^" + ipRange
                         + "\\." + ipRange
                         + "\\." + ipRange
                         + "\\." + ipRange + "$");
    QRegExpValidator *ipValidator = new QRegExpValidator(ipRegex, this);
    ui->host->setValidator(ipValidator);
}
//###################
//#    FUNCTIONS    #
//###################
int MainWindow::WhoIsWidget(QString mac)  //returns the number of widget where mac
{
    for(int i = 0; i < n; i++){
       if(list[i]->getMacAddr() == mac)
          return i;
    }
    return -1;
}
int MainWindow::WhoIsFree()               //returns the empty widget
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
    logFileRead->clear();
    logFileRead->addText(file.readAll());
  }
}
int MainWindow::WhoseIsTimer(QTimer *t)   //return
{
  for(int i = 0; i < n; i++){
      if(t == &timerPing[i])
          return i;
  }
  return -1;
}
/*virtual*/ void MainWindow::closeEvent(QCloseEvent *)
{
    foreach (QTcpSocket* client, map.values()) {
        client->close();
    }
    if(serverListening)
        logging("Server stop");
    logging("Application stop");
    logging("==============================");
    exit(0);
}
void MainWindow::logging(QString text)
{
    loging.append(text);
    appendLogWindow();
}
//###################
//#     SLOTS       #
//###################
void MainWindow::slotStartServer()
{
    if(!serverListening){
        m_ptcpServer = new QTcpServer(this);
        if(ui->port->text().isEmpty()){
            QMessageBox::critical(this,"Ошибка ввода порта","Ошибка ввода номера порта - пустое значение недопустимо!");
            ui->port->setFocus();
            return;
        }
        bool isOk = false;
        int value = ui->port->text().toInt(&isOk);
        if(isOk == false){
            QMessageBox::critical(this,"Ошибка ввода порта","Ошибка ввода номера порта - " +  ui->port->text() + ", не является числовым значением!");
            ui->port->setFocus();
            return;
        }
        if(value < 1 || value > 65535){
            QMessageBox::critical(this,"Ошибка ввода порта","Ошибка ввода номера порта - " +  ui->port->text() + ", выходит за допустимые приделы(1-65535)!");
            ui->port->setFocus();
            return;
        }
// Start server
        if (!m_ptcpServer->listen(QHostAddress(ui->host->text()), ui->port->text().toInt())) {
            QMessageBox::critical(this,"Ошибка сервера","Невозможно запустить сервер: "+ m_ptcpServer->errorString() );
            m_ptcpServer->close();
            return;
        }
        serverListening = true;
        ui->host->setEnabled(false);
        ui->port->setEnabled(false);
        ui->start_listening->setText("Стоп");
        ui->start_listening->setEnabled(true);
        ui->start_listening->setStyleSheet(QString::fromUtf8("background-color: rgb(240, 29, 29);"));
//        Timer_start_stop.start(100);
//        connect(&Timer_start_stop,  SIGNAL(timeout()),        SLOT(slotTimerStartServer()) );
        connect(m_ptcpServer,       SIGNAL(newConnection()),  SLOT(slotNewConnection()) );
        logging("Server start");
        return;
    }
// STOP server
    if(serverListening){
        serverListening = false;
        ui->host->setEnabled(true);
        ui->port->setEnabled(true);
        ui->start_listening->setText("Пуск");
        ui->start_listening->setStyleSheet(QString::fromUtf8("background-color: rgb(63, 199, 72);"));

        logging("Server stop");
        player->stop();
        timerError.stop();
        timerBlink.stop();
        ui->attention->setPixmap(QPixmap(":/attention-bw.png"));
        pStatusBarError->setText("Ошибок нет =))");
        foreach (QTcpSocket* pClientSocket, map.values() ) {
            pClientSocket->close();
        }
        m_ptcpServer->close();
        delete m_ptcpServer;
        return;
    }
}
/*virtual*/ void MainWindow::slotNewConnection()
{
    QThread::usleep(500);
    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();

    if(countClient > MAX_CLIENTS){
        pClientSocket->close();
        emit signalError(4);
        return;
    }

    logging("connecting peer: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) );
    QString tmp = pStatusBarCountClients->text().mid(0,30);
    pStatusBarCountClients->setText(tmp + QString::number(++countClient));

//    connect(pClientSocket, SIGNAL(disconnected()),pClientSocket,  SLOT(deleteLater())     );
    connect(pClientSocket, SIGNAL(disconnected()),this,           SLOT(slotdisconnect())  );
    connect(pClientSocket, SIGNAL(readyRead()),   this,           SLOT(slotReadClient())  );
    connect(pClientSocket, SIGNAL(error(QAbstractSocket::SocketError)),SLOT(slotSocketError(QAbstractSocket::SocketError)) );

    QTimer* pTimer = new QTimer;
    map2[pTimer] = pClientSocket;
    connect(pTimer,        SIGNAL(timeout()), SLOT(slotTimerEmptyClientOut()) );
    pTimer->start(TIME_OUT);
}
void MainWindow::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    map2.key(pClientSocket)->start(TIME_OUT);

    QByteArray answer = pClientSocket->readAll();
//    qDebug() << answer;
    pClientSocket->flush();

    if(answer.startsWith("MAC:")){
        QString answerStr = answer.mid(4,17);
        foreach (QString key, map.keys()) {
              if(answerStr == key){
                  logging("ERROR 1(conflict the key-addresses) Connecting client: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()));
                  emit signalError(1);
                  pClientSocket->close();
                  QThread::usleep(500);
                  return;
              }
        }
        map[answerStr] = pClientSocket;
        int free = WhoIsFree();
        if(free == -1){
            logging("ERROR 3(There is not enough free space on the control panel MO-1) Connecting client: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()));
            emit signalError(3);
            pClientSocket->close();
            map.remove(map.key(pClientSocket));
            return;
        }
        list[free]->enabledHost(true);
        list[free]->setMacAddr(answerStr);

        connect(list[free], SIGNAL(signalPush(char)),   SLOT(slotSendToClient(char))  );
        connect(list[free], SIGNAL(signalReboot(char)), SLOT(slotReboot(char))        );
        connect(list[free], SIGNAL(signalArduinoOpen()),SLOT(slotArduinoGetData()) );

        return;
    }
    int widget = WhoIsWidget(map.key(pClientSocket));
    if(widget != -1 ){
        if(answer.startsWith("low")){
            list[widget]->setVal(0);
            return;
        }
        if(answer.startsWith("high")){
            list[widget]->setVal(1);
            return;
        }
        if(answer.startsWith("ping")){
            disconnect(&timerPing[widget],SIGNAL(timeout()), this, SLOT(slotTimeOut()) );
            timerPing[widget].start(TIME_OUT_PING);
            connect(&timerPing[widget],SIGNAL(timeout()), SLOT(slotTimeOut()) );
            return;
        }
        if(answer.startsWith("cmd_")){
            if(answer.startsWith("cmd_ard+")){
                int widget = WhoIsWidget(map.key(pClientSocket));
                if (widget >= 0){
                    list[widget]->withArduino(true);
                    map3[&timerArd[widget]] = pClientSocket;
                    disconnect(&timerArd[widget],SIGNAL(timeout()), this, SLOT(slotTimeOutArd()) );
                    timerArd[widget].start(TIME_OUT_PING);
                    connect(&timerArd[widget],SIGNAL(timeout()), SLOT(slotTimeOutArd()) );
                }
            }
//            logging("client " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) + " sent : " + QString(answer.data()) );
            return;
        }
    }

    logging("ERROR 2(Error client request) Connecting client: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()));
    emit signalError(2);
//    qDebug() << answer;
    pClientSocket->close();// disconnected();
}
void MainWindow::slotSendToClient(char byte)
{
    Widget* w = (Widget*)sender();
    QTcpSocket* pClientSocket = map.value(w->getMacAddr());
    pClientSocket->write(&byte);
    logging("send " + QString(byte) + " to client: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) );
}
void MainWindow::slotArduinoGetData()
{
  Widget* w = (Widget*)sender();
  QTcpSocket* pClientSocket = map.value(w->getMacAddr());
//  pClientSocket->write(&byte);
}
void MainWindow::slotdisconnect()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QString workingTime = "NOT CORRECT CLIENT";
    if(map.size() > 0){
        map2.key(pClientSocket)->stop();
        int widget = WhoIsWidget(map.key(pClientSocket));
//        qDebug() << "widget " << widget;
        if(widget != -1){
            timerPing[widget].stop();
            workingTime = list[widget]->getWorkingTime();   // если перезагрузка то время работы ноль!!!
            list[widget]->enabledHost(false);
            timerPing[widget].stop();
            timerArd[widget].stop();
            disconnect(list[widget],  SIGNAL(signalPush(char)),   this, SLOT(slotSendToClient(char))  );
            disconnect(list[widget],  SIGNAL(signalReboot(char)), this, SLOT(slotReboot(char))        );
        }
    }

    delete map2.key(pClientSocket);
    map2.remove(map2.key(pClientSocket) );
    map3.remove(map3.key(pClientSocket) );

//    qDebug() << "map2" << map2.keys() << map2.values();
//    qDebug() << "map3 " << map3.keys() << map3.values();

    if(map.size() > 0)
        map.remove(map.key(pClientSocket));

//    qDebug() << "map " << map.keys() << map.values();
    pStatusBarCountClients->setText(pStatusBarCountClients->text().mid(0,30) + QString::number(--countClient));
    logging("disconnecting peer: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) + ", working time: " +  workingTime);
    if(workingTime == "NOT CORRECT CLIENT"){
        logging("ERROR 6(suspicious client) Connecting client: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()));
        emit signalError(6);
    }
    ui->start_listening->setFocus();
    pClientSocket->deleteLater();
}
void MainWindow::slotReboot(char byte)
{
    Widget* w = (Widget*)sender();
    QTcpSocket* pClientSocket = map.value(w->getMacAddr());
    logging("rebooting client: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) );
    pClientSocket->write(&byte);

    int i = WhoIsWidget(w->getMacAddr());
    disconnect(list[i], SIGNAL(signalReboot(char)), this, SLOT(slotReboot(char))        );
    disconnect(list[i], SIGNAL(signalPush(char)),   this, SLOT(slotSendToClient(char))  );

    pClientSocket->close();
    ui->start_listening->setFocus();
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
  connect(logFileRead, SIGNAL(signalClose()), SLOT(slotCloseLog()) );
  connect(logFileRead, SIGNAL(signalClearLogFile()), SLOT(slotClearLogFile()) );
}
void MainWindow::slotCloseErrorList()
{
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
void MainWindow::slotClearLogFile()
{
  logFileRead->clear();
  QFile file(QString(QDir::currentPath().replace("/","\\") + "\\esp-01.log") );
  file.open(QIODevice::WriteOnly);
  file.write("");
  disconnect(logFileRead, SIGNAL(signalClose()), this, SLOT(slotClearLogFile()) );
}

void MainWindow::slotTimeErrorOut()
{
    emit signalError(0);
}
void MainWindow::slotTimerBlinkOut()
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
void MainWindow::slotSocketError(QAbstractSocket::SocketError err)
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();

    QString error = '\0';
    switch (err) {
      case 0:
        error = "The connection was refused by the peer (or timed out).";
        break;
      case 1:
        error = "The remote host closed the connection. Note that the client socket (i.e., this socket) will be closed after the remote close notification has been sent.";
        break;
      case 2:
        error = "The host address was not found.";
        break;
      case 3:
        error = "The socket operation failed because the application lacked the required privileges.";
        break;
      case 4:
        error = "The local system ran out of resources (e.g., too many sockets).";
        break;
        // ДОПИСАТЬ!!!
      default:
        error = "unknow error!";
        break;
      }
    logging("Attention!!!" + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) + ", socket error  - " +  error );
//    qDebug() << err;
}
void MainWindow::slotTimeOut()
{
    QTimer* timer = (QTimer*)(sender());
    int widget = WhoseIsTimer(timer);
    timerPing[widget].stop();
    disconnect(&timerPing[widget],SIGNAL(timeout()), this, SLOT(slotTimeOut()) );
    map.value(list[widget]->getMacAddr())->close();
}
void MainWindow::slotTimeOutArd()
{
//    qDebug() << map3.values() << map3.keys();
//    qDebug() << &timerArd[0];
    QTimer* timer = (QTimer*)(sender());
//    qDebug() << timer;
    timer->stop();
    QTcpSocket* pClientSocket = map3.value(timer);
    int widget = WhoIsWidget(map.key(pClientSocket));
    list[widget]->withArduino(false);
    disconnect(timer,SIGNAL(timeout()), this, SLOT(slotTimeOutArd()) );
//    map3.remove(timer);
}
void MainWindow::slotTimerEmptyClientOut()
{
    QTimer* timer = (QTimer*)(sender());
    disconnect(timer,SIGNAL(timeout()), this, SLOT(slotTimerEmptyClientOut()) );
    logging("ERROR 5 the client " + map2.value(timer)->peerAddress().toString() + ":" + QString::number(map2.value(timer)->peerPort())+  " does not transmit any data");
    emit signalError(5);
    map2.value(timer)->close();
}
void MainWindow::slotError(int err)
{
    ui->attention->setPixmap(QPixmap(":/attention.png"));
    led = true;
    timerError.start(10000);
    timerBlink.start(300);
    player->stop();
    if(err != 0)
        player->play();
    switch (err) {
      case 0:
        player->stop();
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
      case 4:
        pStatusBarError->setText("Ошибка: " + QString::number(err));
        break;
      case 5:
        pStatusBarError->setText("Ошибка: " + QString::number(err));
      case 6:
        pStatusBarError->setText("Ошибка: " + QString::number(err));
        break;
        break;
      default:
        break;
      }
}
MainWindow::~MainWindow()
{
  delete ui;
}
