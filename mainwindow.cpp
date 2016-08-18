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

#define TIME_OUT 5000
#define MAX_CLIENTS 3

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->start_listening->setStyleSheet(QString::fromUtf8("background-color: rgb(63, 199, 72);"));

    serverListening = false;
    led             = false;

    QVBoxLayout *pLayout = new QVBoxLayout;

    for(int i = 0; i < n; i++){
        list[i] = new Widget;
        pLayout->addWidget(list[i]);
    }
    ui->groupBox->adjustSize();

    ui->groupBox->setLayout(pLayout);
    ui->attention->setPixmap(QPixmap(":/attention-bw.png"));

    this->setStatusBar(new QStatusBar);
    pStatusBarError = new QLabel("Ошибок нет =))");
    pStatusBarCountClients = new QLabel("Кол-во подключенных клиентов: 0");
    this->statusBar()->addWidget(pStatusBarCountClients);
    this->statusBar()->addWidget(pStatusBarError);

//    QString fileName = QDir::currentPath() + "\\tmp.mp3";
//    QFile fileQrc(":/attention.mp3");
//    fileQrc.open(QIODevice::ReadOnly);
//    QByteArray ba = fileQrc.readAll();
//    QFile fileTmp(fileName);
//    fileTmp.open(QIODevice::WriteOnly);
//    fileTmp.write(ba);
//    fileTmp.close();
//    fileQrc.close();

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
      if(t == &timer[i])
          return i;
  }
  return -1;
}
/*virtual*/ void MainWindow::closeEvent(QCloseEvent *)
{
    QString fileName = QDir::currentPath() + "\\tmp.mp3";
    QFile file(fileName);
    file.remove();

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
        ui->start_listening->setStyleSheet(QString::fromUtf8("background-color: rgb(240, 29, 29);"));
        connect(m_ptcpServer, SIGNAL(newConnection()), SLOT(slotNewConnection()) );
        logging("Server start");
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
        logging("Server stop");
        player->stop();
        timerError.stop();
        timerBlink.stop();
        ui->attention->setPixmap(QPixmap(":/attention-bw.png"));
        pStatusBarError->setText("Ошибок нет =))");
        return;
    }
}
/*virtual*/ void MainWindow::slotNewConnection()
{
    QThread::usleep(500);
    QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();
    logging("connecting peer: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) );
    QString tmp = pStatusBarCountClients->text().mid(0,30);
    pStatusBarCountClients->setText(tmp + QString::number(++countClient));

    connect(pClientSocket, SIGNAL(disconnected()),pClientSocket,  SLOT(deleteLater())     );
    connect(pClientSocket, SIGNAL(disconnected()),this,           SLOT(slotdisconnect())  );
    connect(pClientSocket, SIGNAL(readyRead()),   this,           SLOT(slotReadClient())  );
    connect(pClientSocket, SIGNAL(error(QAbstractSocket::SocketError)),SLOT(slotSocketError(QAbstractSocket::SocketError)) );

    QTimer* timer = new QTimer;
    map2[timer] = pClientSocket;
    timer->start(TIME_OUT);
    connect(timer, SIGNAL(timeout()), SLOT(slotTimerEmptyClientOut()) );

    if(countClient > MAX_CLIENTS){
        pClientSocket->close();
        emit signalError(4);
    }
}
void MainWindow::slotReadClient()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QByteArray answer = pClientSocket->readAll();
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
        return;
    }
    int w = WhoIsWidget(map.key(pClientSocket));
    if(w != -1 ){
        map2.key(pClientSocket)->start(TIME_OUT);
        if(answer.startsWith("low")){
            list[w]->setVal(0);
            return;
        }
        if(answer.startsWith("high")){
            list[w]->setVal(1);
            return;
        }
        if(answer.startsWith("ping")){
            disconnect(&timer[w],SIGNAL(timeout()), this, SLOT(slotTimeOut()) );
            timer[w].start(TIME_OUT);
            connect(&timer[w],SIGNAL(timeout()), SLOT(slotTimeOut()) );
            return;
        }
    }
    if(answer.startsWith("cmd_")){
        logging("client " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) + " sent : " + QString(answer.data()) );
        return;
    }
    logging("ERROR 2(Error client request) Connecting client: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()));
    emit signalError(2);
    pClientSocket->close();// disconnected();
}
void MainWindow::slotSendToClient(char byte)
{
    Widget* w = (Widget*)sender();
    QTcpSocket* pClientSocket = map.value(w->getMacAddr());
    pClientSocket->write(&byte);
    logging("send " + QString(byte) + " to client: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) );
}
void MainWindow:: slotdisconnect()
{
    QTcpSocket* pClientSocket = (QTcpSocket*)sender();

    map2.key(pClientSocket)->stop();
    disconnect(timer,SIGNAL(timeout()), this, SLOT(slotTimerEmptyClientOut()) );
    delete map2.key(pClientSocket);
    map2.remove(map2.key(pClientSocket) );

    QString workingTime = "NOT CORRECT CLIENT";
    if(map.size() > 0){
        int i = WhoIsWidget(map.key((QTcpSocket*)sender()));
        if(i != -1){
            workingTime = list[i]->getWorkingTime();
            list[i]->enabledHost(false);
            timer[i].stop();
            map.remove(map.key(pClientSocket));
            disconnect(list[i],  SIGNAL(signalPush(char)),   this, SLOT(slotSendToClient(char))  );
            disconnect(list[i],  SIGNAL(signalReboot(char)), this, SLOT(slotReboot(char))        );
        }
    }
    QString tmp = pStatusBarCountClients->text().mid(0,30);
    pStatusBarCountClients->setText(tmp + QString::number(--countClient));
    logging("disconnecting peer: " + pClientSocket->peerAddress().toString() + ":" + QString::number(pClientSocket->peerPort()) + ", working time: " +  workingTime);
    ui->start_listening->setFocus();
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
    qDebug() << err;
}
void MainWindow::slotTimeOut()
{
    QTimer* timer = (QTimer*)(sender());
    timer->stop();
    map.value(list[WhoseIsTimer(timer)]->getMacAddr())->close();
    disconnect(timer,SIGNAL(timeout()), this, SLOT(slotTimeOut()) );
}
void MainWindow::slotTimerEmptyClientOut()
{
    QTimer* timer = (QTimer*)(sender());
    timer->stop();
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
        break;
      default:
        break;
      }
}
MainWindow::~MainWindow()
{
  delete ui;
}
