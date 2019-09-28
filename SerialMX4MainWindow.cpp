#include "SerialMX4MainWindow.h"
#include "ui_serial-mx4.h"

#include <QSerialPortInfo>
#include <QComboBox>

namespace {
float t_fval_o = .0;
}

SerialMX4MainWindow::SerialMX4MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SerialMX4MainWindow),
    transactionCount(0),
    rmx4(new RaytekMX4)
{
    ui->setupUi(this);
//    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
//        ui->ComPort_ComboBox->addItem(info.portName());

//    t = new QTimer(this);                           // Создать таймер

    connect( ui->pb_Connect,    SIGNAL(clicked()),this, SLOT(onConnectButtonPushed())   );
    connect( ui->pb_Disconnect, SIGNAL(clicked()),this, SLOT(onDisconnectButtonPushed()));
    connect( ui->pb_UI0,        SIGNAL(clicked()),rmx4, SLOT(SetUserInterfaceOff())     );
    connect( ui->pb_UI1,        SIGNAL(clicked()),rmx4, SLOT(SetUserInterfaceOn())      );
    connect( ui->pb_BM0,        SIGNAL(clicked()),rmx4, SLOT(StopBurstMode())           );
    connect( ui->pb_BM1,        SIGNAL(clicked()),rmx4, SLOT(StartBurstMode())          );
    connect( ui->pb_SetBurstString, SIGNAL(clicked()),rmx4, SLOT(SetBurstString())      );
    connect( ui->pb_AskT,       SIGNAL(clicked()),rmx4, SLOT(AskTemp())                 );

    connect( rmx4,SIGNAL(error_mx4(QString)),   this,SLOT(processError(QString))    );
    connect( rmx4,SIGNAL(otvet_mx4(QString)),   this,SLOT(showResponse(QString))    );
    connect( rmx4,SIGNAL(timeout_mx4(QString)), this,SLOT(processTimeout(QString))  );

    /* ~~~~~~~~~~~~~~~~~~~~~~~~ -> QCustomPlot ~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    ui->tPlot->addGraph();                                              // График 1. Температура ИК
    ui->tPlot->addGraph();                                              // График 2. Температура пробника
    ui->tPlot->graph(0)->setPen(QPen(Qt::blue));                        // График 1 - синий
    ui->tPlot->graph(1)->setPen(QPen(Qt::red));                         // График 2 - красный
    //ui->tPlot->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
    ui->tPlot->graph(0)->setBrush(Qt::NoBrush);                         // Цвет заливки - пустой
    ui->tPlot->graph(0)->setChannelFillGraph(ui->tPlot->graph(1));      // Заливка между графиками 1 и 2 (или осью x)
    ui->tPlot->graph(0)->setAntialiasedFill(false);                     // Не сглаживать
    ui->tPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);            // Засечки дата/время
    ui->tPlot->xAxis->setDateTimeFormat("hh:mm:ss");                    // Формат засечек
    ui->tPlot->xAxis->setAutoTickStep(false);                           // Не определять шаг засечек автоматически
    ui->tPlot->xAxis->setTickStep(1);                                   // Шаг засечек - "1"
    ui->tPlot->axisRect()->setupFullAxesBox();                          // Создать оси
    ui->tPlot->yAxis->setRangeLower(-50.0);
    ui->tPlot->yAxis->setRangeUpper(+50.0);
    // make left and bottom axes transfer their ranges to right and top axes:
//    connect(ui->tPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->tPlot->xAxis2, SLOT(setRange(QCPRange)));
//    connect(ui->tPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->tPlot->yAxis2, SLOT(setRange(QCPRange)));
    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
//    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
//    dataTimer.start(0); // Interval 0 means to refresh as fast as possible
    /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~ QCustomPlot -> ~~~~~~~~~~~~~~~~~~~~~~~~ */

    /**/
    Q_FOREACH(QSerialPortInfo port, QSerialPortInfo::availablePorts()) {
      ui->cb_SerialPort->addItem(port.portName());
      ui->cb_SerialPort_2->addItem(port.portName());
    }
}

SerialMX4MainWindow::~SerialMX4MainWindow()
{
    delete rmx4;
    delete ui;
}

void SerialMX4MainWindow::onConnectButtonPushed()    // По нажатию кнопки подключения к serial-порту
{
//    t->start(100);             // Запустить таймер с интервалом 1 секунда

    QString fname =
        QDate::currentDate().toString("yyyy-MM-dd") +
        "_" +
        QTime::currentTime().toString("hh-mm") +
        "_REC.TXT";
    f.setFileName(fname);
    if( !f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append) )
    {
        processError(tr("Ошибка открытия файла для чтения"));
        return;
    }
    fout.setDevice(&f);
    ui->pb_Connect->setEnabled(false);
    ui->pb_Disconnect->setEnabled(true);
    ui->Console_plainTextEdit->appendPlainText(tr("onConnectButtonPushed"));
}

void SerialMX4MainWindow::onDisconnectButtonPushed()    // По нажатию кнопки подключения к serial-порту
{
//    if( t->isActive() )             // Таймер уже работает?
//    {
        if( f.isOpen() )            // Файл телеметрии закрыт?
        {
            f.close();
        }
//        t->stop();                  // Остановить таймер
//    }
    ui->pb_Connect->setEnabled(true);
    ui->pb_Disconnect->setEnabled(false);
    ui->Console_plainTextEdit->appendPlainText(tr("onDisconnectButtonPushed"));
}

void SerialMX4MainWindow::showResponse(const QString &s)
{
    QString answer(s);// = rmx4->parse(s);

    ++transactionCount;

    ui->Console_plainTextEdit->appendPlainText(tr("showResponse#%1: %2").arg(transactionCount).arg(s));

//    ui->AnswerEdit->setText(answer);

    if( rmx4->BurstRunning() )
    {
        /* BURST MODE */
        if( answer.size() > 12 )
        {
            int index = answer.indexOf('T');
            if( index > 0 )
            {
                answer.remove(0,index);
            }
        }
        if( !answer.startsWith("T") )   // $=TX
        {
            return;
        }
        answer.replace(QString("\r\n"), QString(" "), Qt::CaseInsensitive);
        QStringList list_answer = answer.split(" ");
        int i = 0; int t_count = 0; int x_count = 0;
        float t_fval = 0.0; float x_fval = 0.0;
        bool ok;
        foreach( QString str, list_answer )
        {
            // qDebug() << str;
            float tmp;
            if( str.startsWith("T") )   // Например, "T022.8"
            {
                str.remove("T");
                if( str.size() == 5 )   // Например, "022.8" - пять символов
                {
                    tmp = str.toFloat(&ok);
                    if( ok )
                    {
                        t_fval += tmp;
                        ++t_count;
                    }
                }
            }
            if( str.startsWith("X") )   // Например, "X022.8"
            {
                str.remove("X");
                if( str.size() == 5 )   // Например, "022.8" - пять символов
                {
                    tmp = str.toFloat(&ok);
                    if( ok )
                    {
                        x_fval += tmp;
                        ++x_count;
                    }
                }
            }
        }
        /* Усреднение */
        if( t_count > 0 && x_count > 0 )
        {
            t_fval /= t_count;
            x_fval /= x_count;
            QString t_qstr = QString::number(t_fval,'f',1);
            QString x_qstr = QString::number(x_fval,'f',1);
            if( f.isOpen() )
            {
                fout << QTime::currentTime().toString("hh:mm:ss.zzz") << " " << t_qstr << " " << x_qstr << endl;
            }
            ui->lineEdit_TempIK->setText(t_qstr);
            ui->lineEdit_TempTP->setText(x_qstr);
            double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
            // add data to lines:
            ui->tPlot->graph(0)->addData(key,t_fval);
            ui->tPlot->graph(1)->addData(key,x_fval);
            // remove data of lines that's outside visible range:
            ui->tPlot->graph(0)->removeDataBefore(key-8);
            ui->tPlot->graph(1)->removeDataBefore(key-8);
            // rescale value (vertical) axis to fit the current data:
//            ui->tPlot->graph(0)->rescaleValueAxis();
            // make key axis range scroll with the data (at a constant range size of 8):
            ui->tPlot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
            ui->tPlot->replot();
        }
    }
    else {
        if( !answer.startsWith("!") )
        {
            return;
        }
        answer.remove("!");
        if( answer.startsWith("T") )
        {
            answer.remove("T");
            ui->lineEdit_TempIK->setText( answer );
        }
    }

//    if( answer.startsWith("T") )
//    {
//        answer.remove(QChar('T'), Qt::CaseInsensitive);
//        QStringList list1 = answer.split("\r\n");
//        int i = 0;
//        float t_fval = 0.0;
//        bool ok;
//        foreach( QString str, list1 )
//        {
//            if( str.size() == 5 )   // Например, "022.8" - пять символов
//            {
//                // qDebug() << str;
//                float tmp = str.toFloat(&ok);
//                if( ok )
//                {
//                    t_fval += tmp;
//                    ++i;
//                }
//            }
//        }
//        if( i > 0 ) {
//            t_fval /= i;
//            QString t_qstr = QString::number(t_fval,'f',1);
//            if( f.isOpen() )
//            {
//                fout << QTime::currentTime().toString("hh:mm:ss.zzz") << " " << t_qstr << endl;
//            }
//            ui->lineEdit_TempIK->setText(t_qstr);
//        }
//        if( rmx4->BurstRunning() )
//        {
//            double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
//            // add data to lines:
//            ui->tPlot->graph(0)->addData(key,t_fval);
//            // remove data of lines that's outside visible range:
//            ui->tPlot->graph(0)->removeDataBefore(key-8);
//            // rescale value (vertical) axis to fit the current data:
////            ui->tPlot->graph(0)->rescaleValueAxis();
//            // make key axis range scroll with the data (at a constant range size of 8):
//            ui->tPlot->xAxis->setRange(key+0.25, 8, Qt::AlignRight);
//            ui->tPlot->replot();
//        }
//    }
}

void SerialMX4MainWindow::processError(const QString &s)
{
    //t->stop();
    ui->Console_plainTextEdit->appendPlainText(tr("processError"));
    ui->Console_plainTextEdit->appendPlainText(s);
}

void SerialMX4MainWindow::processTimeout(const QString &s)
{
    ui->Console_plainTextEdit->appendPlainText(tr("processTimeout"));
    ui->Console_plainTextEdit->appendPlainText(s);
}
