/****************************************************************************
**
** Copyright (C) 2012 Denis Shienkov <denis.shienkov@gmail.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "io_mx4.h"

#include <QtSerialPort/QSerialPort>

#include <QTime>

namespace {
QString SerialPortSetup(QSerialPort& port, QString portName)
{
    port.close();
    port.setPortName(portName);

    if (!port.open(QIODevice::ReadWrite)) {
        return QObject::tr("Can't open %1, error code %2").arg(portName).arg(port.error());
    }
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Communication reference of Raynger MX4 (Firmware Revision 1.06)
 * RS232 interface parameters:
 * Baudrate:	9600
 * Data bits:	8
 * Parity:		none
 * Stop bits:	1
 * Protocol:	none
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
    if (!port.setBaudRate(QSerialPort::Baud9600)) {
        return QObject::tr("Can't set baud rate 9600 baud to port %1, error code %2")
                .arg(portName).arg(port.error());
    }

    if (!port.setDataBits(QSerialPort::Data8)) {
        return QObject::tr("Can't set 8 data bits to port %1, error code %2")
                .arg(portName).arg(port.error());
    }

    if (!port.setParity(QSerialPort::NoParity)) {
        return QObject::tr("Can't set no parity to port %1, error code %2")
                .arg(portName).arg(port.error());
    }

    if (!port.setStopBits(QSerialPort::OneStop)) {
        return QObject::tr("Can't set 1 stop bit to port %1, error code %2")
                .arg(portName).arg(port.error());
    }

    if (!port.setFlowControl(QSerialPort::NoFlowControl)) {
        return QObject::tr("Can't set no flow control to port %1, error code %2")
                .arg(portName).arg(port.error());
    }

    return QString();   // isEmpty, isNull
}
}//namespace

QT_USE_NAMESPACE

MX4_THREAD::MX4_THREAD(QObject *parent)
    : QThread(parent), _waitWriteTimeout(1000),_waitReadTimeout(1000), quit(false)
{
    //BURST_MODE = false;
}

MX4_THREAD::~MX4_THREAD()
{
    stop_otkl();
}

void MX4_THREAD::stop_otkl()
{
    mutex.lock();
    quit = true;
    cond.wakeOne();
    mutex.unlock();
    wait();                     // Дождаться завершения Run()
}

void XCHANGE_THREAD::zapros(const QString &request)
{
    QMutexLocker locker(&mutex);
    this->_portName = "COM1";//portName;
    this->_waitWriteTimeout = 1000;//waitWriteTimeout;
    this->_waitReadTimeout = 1000;//waitReadTimeout;
    this->_request = request;
    this->quit = false;
    if (!isRunning())
        start();
    else
        cond.wakeOne();
}

void BM_THREAD::zapusk()
{
    QMutexLocker locker(&mutex);
    this->_portName = "COM1";//portName;
    this->_waitWriteTimeout = 1000;//waitWriteTimeout;
    this->_waitReadTimeout = 1000;//waitReadTimeout;
    //this->_request = request;
    //this->quit = !START;
    this->quit = false;
    if (!isRunning())
        start();
    else
        cond.wakeOne();
}

void BM_THREAD::run()
{
    bool currentPortNameChanged = false;
    bool BM1 = false;

    mutex.lock();
    QString currentPortName;
    if (currentPortName != _portName) {
        currentPortName = _portName;
        currentPortNameChanged = true;
    }

    int WriteTimeout = _waitWriteTimeout;
    int ReadTimeout = _waitReadTimeout;
    //QString Request = _request;
    mutex.unlock();

    QSerialPort serial;

    while( !quit )    // Режим непрерывного чтения из порта
    {
        /* Настройка serial-порта */
        if( currentPortNameChanged )
        {
            QString errorSerialOpenText = SerialPortSetup(serial,currentPortName);
            if( errorSerialOpenText.isEmpty() )
            {
                emit error(QString(tr("Connected to port %1").arg(currentPortName)));
            }
            else {
                emit error(errorSerialOpenText);
                return;
            }
        }
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if( !BM1 )
        {
            QByteArray requestData = QString("BM=1\r\n").toLocal8Bit();
            serial.write(requestData);                      // Отправить запрос в порт
            if( !serial.waitForBytesWritten(WriteTimeout) ) // Дождаться окончания отправки запроса в порт
            {
                emit timeout(tr("waitForBytesWritten timeout %1")
                             .arg(QTime::currentTime().toString()));
            }
            BM1 = true;
        }

        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        /* Чтение ответа из serial-порта */
        if (serial.waitForReadyRead(ReadTimeout)) {         // Дождаться ответа
            QByteArray responseData = serial.readAll();     // Считать содержимое буфера
//            while (serial.waitForReadyRead(ReadTimeout))    // !!! 10 мс?
//                responseData += serial.readAll();           // !!! Проверить, есть ли необходимость читать далее

            QString response(responseData);
            emit this->otvet(response);
        } else {
            emit timeout(tr("waitForReadyRead timeout %1")
                         .arg(QTime::currentTime().toString()));
        }
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

        mutex.lock();
        cond.wait(&mutex,250);                      // Ждать 250 мс
        if (currentPortName != _portName) {
            currentPortName = _portName;
            currentPortNameChanged = true;
        } else {
            currentPortNameChanged = false;
        }
//        WaitTimeout = _waitWriteTimeout;
        ReadTimeout = _waitReadTimeout;
        //Request = _request;
        mutex.unlock();
    }

    /* По выходу из цикла чтения дать команду останова на ИК-термометр */
    QString Request = "ESC";
    QByteArray requestData = Request.toLocal8Bit(); // Отправить ESC
    serial.write(requestData);                      // Отправить запрос в порт
    if( !serial.waitForBytesWritten(WriteTimeout) ) // Дождаться окончания отправки запроса в порт
    {
        emit timeout(tr("waitForBytesWritten timeout %1")
                     .arg(QTime::currentTime().toString()));
    }
    emit otvet(tr("ESC Burst Mode from %1").arg(currentPortName));
}

void XCHANGE_THREAD::run()
{
    bool currentPortNameChanged = false;
//    bool CONTINUOUS_READ = false;

    mutex.lock();
    QString currentPortName;
    if (currentPortName != _portName) {
        currentPortName = _portName;
        currentPortNameChanged = true;
    }

    int WriteTimeout = _waitWriteTimeout;
    int ReadTimeout = _waitReadTimeout;
    QString Request = _request;
//    CONTINUOUS_READ = BURST_MODE;
    mutex.unlock();

    QSerialPort serial;

    while (!quit) {
        /* Настройка serial-порта */
        if( currentPortNameChanged )
        {
            QString errorSerialOpenText;
            errorSerialOpenText = SerialPortSetup(serial,currentPortName);
            if( errorSerialOpenText.isEmpty() )
            {
                emit error(QString(tr("Connected to port %1").arg(currentPortName)));
            }
            else {
                emit error(errorSerialOpenText);
                return;
            }
        }

        /* Отправка запроса в serial-порт */
        QByteArray requestData = Request.toLocal8Bit();
        serial.write(requestData);                      // Отправить запрос в порт
        if( !serial.waitForBytesWritten(WriteTimeout) ) // Дождаться окончания отправки запроса в порт
        {
            emit timeout(tr("waitForBytesWritten timeout %1")
                         .arg(QTime::currentTime().toString()));
        }
        /* Чтение ответа из serial-порта (пробовать, даже если при записи была ошибка) */
        if (serial.waitForReadyRead(ReadTimeout)) {         // Дождаться ответа
            QByteArray responseData = serial.readAll();     // Считать содержимое буфера
            while (serial.waitForReadyRead(ReadTimeout))    // !!! 10 мс?
                responseData += serial.readAll();           // !!! Проверить, есть ли необходимость читать далее

            QString response(responseData);
            emit this->otvet(response);
        } else {
            emit timeout(tr("waitForReadyRead timeout %1")
                         .arg(QTime::currentTime().toString()));
        }
        mutex.lock();
        cond.wait(&mutex);                              // Ждать, пока не разбудят очередным запросом
        if (currentPortName != _portName) {
            currentPortName = _portName;
            currentPortNameChanged = true;
        } else {
            currentPortNameChanged = false;
        }
//        WaitTimeout = _waitWriteTimeout;
        ReadTimeout = _waitReadTimeout;
        Request = _request;
        mutex.unlock();
    }
}
