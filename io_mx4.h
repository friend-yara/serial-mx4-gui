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

#ifndef IO_MX4_H
#define IO_MX4_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTimer>

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                          Базовый класс
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class MX4_THREAD : public QThread
{
    Q_OBJECT

public:
    MX4_THREAD(QObject *parent = 0);
    ~MX4_THREAD();

    virtual void run() = 0;
    void stop_otkl();

signals:
    void otvet(const QString &s);   // Ответ от MX4 Ranger
    void error(const QString &s);   // Ошибка при открытии/настройке serial-порта
    void timeout(const QString &s); // Превышен таймаут чтения/записи serial-порта

protected:
    QString _portName;              // Имя устройства (serial-порта)
    QString _request;               // Строка запроса на устройство
    int _waitWriteTimeout;          // Допустимое время ожидания операции записи в serial-порт
    int _waitReadTimeout;           // Допустимое время ожидания операции чтения из serial-порт
    QMutex mutex;
    QWaitCondition cond;
    bool quit;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                          Режим "Запрос-Ответ"
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class XCHANGE_THREAD : public MX4_THREAD   // Режим "Запрос-Ответ"
{
    Q_OBJECT

public:
//    XCHANGE_THREAD(QObject *parent = 0);
//    ~XCHANGE_THREAD();

    void zapros(const QString &request);
    void run();
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//          Burst Mode (режим чтения температуры по команде BM=1)
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class BM_THREAD : public MX4_THREAD        // Burst Mode (режим чтения температуры по команде BM=1
{
    Q_OBJECT

public:
//    BM_THREAD(QObject *parent = 0);
//    ~BM_THREAD();

    void zapusk();
    void run();

private:
    //bool START_READ;                // Режим выдачи температуры в заданном формате с интервалом 31.25 мс (чтобы выключить BURST MODE, необходимо отправить "ESC")
};

#endif // IO_MX4_H
