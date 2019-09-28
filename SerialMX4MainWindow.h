#ifndef IR_SERIAL_UI_H
#define IR_SERIAL_UI_H

#include <QMainWindow>

#include <QDateTime>
#include <QFile>
#include <QTextStream>

#include "raytek_mx4.h"

namespace Ui {
class SerialMX4MainWindow;
}

class SerialMX4MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SerialMX4MainWindow(QWidget *parent = 0);
    ~SerialMX4MainWindow();

private:
    Ui::SerialMX4MainWindow *ui;

    bool WantToStop;
    int transactionCount;   // Количество отправленных команд/запросов
    QTimer *t;              // Таймер, задающий частоту считывания из serial-порта
    QFile f;                // Файл телеметрии
    QTextStream fout;       // Поток (stream) для вывода данных в файл телеметрии
    RaytekMX4 *rmx4;        // Объект "ИК-термометр"

private slots:
    void onConnectButtonPushed();
    void onDisconnectButtonPushed();
    void showResponse(const QString &s);
    void processError(const QString &s);
    void processTimeout(const QString &s);
};

#endif // IR_SERIAL_UI_H
