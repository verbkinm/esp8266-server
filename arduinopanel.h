#ifndef ARDUINOPANEL_H
#define ARDUINOPANEL_H

#include <QWidget>

namespace Ui {
  class ArduinoPanel;
}

class ArduinoPanel : public QWidget
{
  Q_OBJECT

public:
  explicit ArduinoPanel(QObject *prt);
  ~ArduinoPanel();

 virtual void closeEvent(QCloseEvent*);

signals:
  void signalClose();

private:
  Ui::ArduinoPanel *ui;
  QObject *parent;
};

#endif // ARDUINOPANEL_H
