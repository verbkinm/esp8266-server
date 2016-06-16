#include "widget.h"
#include "ui_widget.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QTime>
#include <QLabel>
#include <QThread>

Widget::Widget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::Widget)
{
  ui->setupUi(this);
}
void Widget::enabledHost(bool b)
{
    ui->macaddr->setEnabled(b);
    ui->on_off->setEnabled(b);
    ui->reboot->setEnabled(b);
    if(b){
        ui->ping->setText("связь");
        QPalette tmp = ui->ping->palette();
        tmp.setColor(QPalette::Text, Qt::green);
        ui->ping->setPalette(tmp);
    }
    if(!b){
        ui->ping->setText("обрыв");
        QPalette tmp = ui->ping->palette();
        tmp.setColor(QPalette::Text, Qt::red);
        ui->ping->setPalette(tmp);
        ui->macaddr->setText("00:00:00:00:00:00");
        ui->on_off->setText("?");
    }
    connect(ui->on_off, SIGNAL(clicked(bool)), SLOT(slot_on_off()) );
    connect(ui->reboot,SIGNAL(clicked(bool)), SLOT(slotReboot()) );
}
const QString Widget::getMacAddr()
{
  return ui->macaddr->text();
}
const QString Widget::getPing(){
  return ui->ping->text();
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
//    if(ui->on_off->text() != "0" || ui->on_off->text() != "1" ){
//        QMessageBox::critical(0,"Ошибка устройства УР-1!","Невозможно получить ответ от подключенного устройства!" );
//    }
}
void Widget::slotReboot()
{
  emit signalReboot('2');
}

Widget::~Widget()
{
  delete ui;
}
