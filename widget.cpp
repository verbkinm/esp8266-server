#include "widget.h"
#include "ui_widget.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QTime>
#include <QLabel>
#include <QThread>

Widget::Widget(int nPort, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Widget)
{
  ui->setupUi(this);
  m_ptcpServer = new QTcpServer(this);
      if (!m_ptcpServer->listen(QHostAddress::Any, nPort)) {
          QMessageBox::critical(0,
                                "Server Error",
                                "Unable to start the server:"
                                + m_ptcpServer->errorString()
                               );
          m_ptcpServer->close();
          return;
      }
    connect(m_ptcpServer, SIGNAL(newConnection()),SLOT(slotNewConnection()) );
    connect(ui->on_off,   SIGNAL(clicked(bool)),  SLOT(slotOnOff())         );
    connect(ui->reboot,   SIGNAL(clicked(bool)),  SLOT(slotReboot())        );
}
void Widget::slotReboot()
{
  sendToClient(map.value(ui->macaddr->text()),'2');
}

/*virtual*/ void Widget::slotNewConnection()
{
  QTcpSocket* pClientSocket = m_ptcpServer->nextPendingConnection();
  connect(pClientSocket, SIGNAL(disconnected()),pClientSocket,  SLOT(deleteLater())     );
  connect(pClientSocket, SIGNAL(disconnected()),this,           SLOT(slotdisconnect())  );
  connect(pClientSocket, SIGNAL(readyRead()),   this,           SLOT(slotReadClient())  );
}
void Widget::slotReadClient()
{
    ui->macaddr->setEnabled(true);
    ui->on_off->setEnabled(true);
    ui->reboot->setEnabled(true);

    QTcpSocket* pClientSocket = (QTcpSocket*)sender();
    QByteArray answer = pClientSocket->readAll();
    if(answer.startsWith("MAC:")){
        ui->macaddr->setText(QString(answer).mid(4,20));
        map[QString(answer).mid(4,20)] = pClientSocket;
        connect(&timer,       SIGNAL(timeout()), SLOT(slotTimeOut()) );
        ui->ping->setText("online");
        QPalette tmp = ui->ping->palette();
        tmp.setColor(QPalette::Text, Qt::green);
        ui->ping->setPalette(tmp);
        return;
    }
    if(answer == "low"){
        ui->on_off->setText("0");
        return;
    }
    if(answer == "high"){
        ui->on_off->setText("1");
        return;
    }
    if(answer == "ping"){
      timer.start(5000);
      return;
    }
}
void Widget::sendToClient(QTcpSocket* pSocket, const char str)
{
    pSocket->write(&str);
}
void Widget::slotOnOff()
{
  if(ui->on_off->text() == "1")
  {
      ui->on_off->setText("0");
      sendToClient(map.value(ui->macaddr->text()),'0');
      return;
  }
  if(ui->on_off->text() == "0")
  {
      ui->on_off->setText("1");
      sendToClient(map.value(ui->macaddr->text()),'1');
      return;
  }
}
void Widget::slotdisconnect()
{

}
void Widget::slotTimeOut()
{
  ui->macaddr->setEnabled(false);
  ui->on_off->setEnabled(false);
  ui->reboot->setEnabled(false);
  ui->on_off->setText("?");
  ui->ping->setText("offline");
  QPalette tmp = ui->ping->palette();
  tmp.setColor(QPalette::Text, Qt::red);
  ui->ping->setPalette(tmp);
  map.value(ui->macaddr->text())->disconnected();
  timer.stop();
}

Widget::~Widget()
{
  delete ui;
}
