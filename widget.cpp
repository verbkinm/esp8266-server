#include "widget.h"
#include "ui_widget.h"
#include "ui_arduinopanel.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QLabel>
#include <QDebug>

Widget::Widget(QObject *prt) :
  QWidget(),isOpen(false),
  ui(new Ui::Widget)
{
  parent = prt;
  qDebug() << parent;

  ui->setupUi(this);
  connect(ui->on_off, SIGNAL(clicked(bool)), SLOT(slot_on_off()) );
  connect(ui->reboot, SIGNAL(clicked(bool)), SLOT(slotReboot())  );
  connect(&timer,     SIGNAL(timeout()),     SLOT(slotTimerOut()));
  connect(ui->bArduino_panel, SIGNAL(clicked(bool)),SLOT(slotArduinoOpen()) );
}
void Widget::enabledHost(bool b)
{
    ui->macaddr->setEnabled(b);
    ui->on_off->setEnabled(b);
    ui->reboot->setEnabled(b);
    ui->timeWork->setEnabled(b);
    if(b){
        ui->ping->setText("связь");
        QPalette tmp = ui->ping->palette();
        tmp.setColor(QPalette::Text, Qt::green);
        ui->ping->setPalette(tmp);
        timer.start(10);
    }
    if(!b){
        ui->ping->setText("обрыв");
        QPalette tmp = ui->ping->palette();
        tmp.setColor(QPalette::Text, Qt::red);
        ui->ping->setPalette(tmp);
        ui->macaddr->setText("00:00:00:00:00:00");
        ui->on_off->setText("?");
        timer.stop();
        timeWork = 0;
        ui->timeWork->setText("00:00:00:00:00");

        withArduino(false);
    }
}
void Widget::withArduino(bool b)
{
  if(b){
    ui->ard->setText("ARD+");
    QPalette tmp = ui->ard->palette();
    tmp.setColor(QPalette::Text, Qt::green);
    ui->ard->setPalette(tmp);
    if(!isOpen)
      ui->bArduino_panel->setEnabled(b);
  }
  if(!b){
    ui->ard->setText("ARD-");
    QPalette tmp = ui->ard->palette();
    tmp.setColor(QPalette::Text, Qt::red);
    ui->ard->setPalette(tmp);

    ui->bArduino_panel->setEnabled(b);
  }
}

const QString Widget::getMacAddr()
{
  return ui->macaddr->text();
}
const QString Widget::getPing(){
  return ui->ping->text();
}
QString Widget::getWorkingTime()
{
    return ui->timeWork->text();
}

void Widget::setMacAddr(QString mac)
{
    ui->macaddr->setText(mac);
}
void Widget::setVal(int bit)
{
    if(bit == -1){
        ui->on_off->setText("?");
        return;
    }
    ui->on_off->setText(QString::number(bit));
}
void Widget::slot_on_off()
{
    if(ui->on_off->text() == "0"){
        emit signalPush('1');
        return;
    }
    if(ui->on_off->text() == "1"){
        emit signalPush('0');
        return;
    }
    if(ui->on_off->text() != "0" || ui->on_off->text() != "1" ){
        QMessageBox::critical(this,"Ошибка устройства УР-1!","Невозможно получить ответ от подключенного устройства!" );
        return;
    }
}
void Widget::slotReboot()
{
  emit signalReboot('2');
}
void Widget::slotTimerOut()
{
    int d = 0;
    int h = 0;
    int m = 0;
    int s = 0;
    int ms= 0;

    timeWork++;
    if(timeWork < 100){
        this->setTimer(0,0,0,0,timeWork);
        return;
    }
    if(timeWork > 100){
        ms = QString::number(timeWork).mid(QString::number(timeWork).length()-2,-1).toInt();
        s = QString::number(timeWork).mid(0,QString::number(timeWork).length()-2).toInt();
        if(s > 59){
            m = s / 60;
            s = s % 60;
        }
        if(m > 59){
            h = m / 60;
            m = m % 60;
        }
        if(h > 23){
            d = h / 24;
            h = h % 24;
        }
        setTimer(d,h,m,s,ms);
    }
}
void Widget::setTimer(int d, int h, int m, int s, int ms)
{
    QString sD,sH,sM,sS,sMs;
    if(d < 10)
        sD = "0" + QString::number(d);
    else
        sD = QString::number(d);
    if(h < 10)
        sH = "0" + QString::number(h);
    else
        sH = QString::number(h);
    if(m < 10)
        sM = "0" + QString::number(m);
    else
        sM = QString::number(m);
    if(s < 10)
        sS = "0" + QString::number(s);
    else
        sS = QString::number(s);
    if(ms < 10)
        sMs = "0" + QString::number(ms);
    else
        sMs = QString::number(ms);
    ui->timeWork->setText(sD+":"+sH+":"+sM+":"+sS+":"+sMs);
}
void Widget::slotArduinoOpen()
{
//    emit signalSendOther('5');

    emit signalArduinoOpen();
    isOpen = true;
    arduino_panel = new ArduinoPanel(parent);
    arduino_panel->show();
    ui->bArduino_panel->setEnabled(false);

    connect(arduino_panel, SIGNAL(signalClose()), SLOT(slotArduinoClose()) );
}
void Widget::slotArduinoClose()
{
  isOpen = false;
  disconnect(arduino_panel, SIGNAL(signalClose()), this, SLOT(slotArduinoClose()) );
  delete arduino_panel;
  ui->bArduino_panel->setEnabled(true);
}

Widget::~Widget()
{
  delete ui;
}
