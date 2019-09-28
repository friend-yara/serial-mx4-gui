#-------------------------------------------------
#
# Project created by QtCreator 2015-04-08T16:50:51
#
#-------------------------------------------------

QT       += core gui

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

greaterThan(QT_MAJOR_VERSION, 4) {
    QT       += widgets serialport printsupport
} else {
    include($$QTSERIALPORT_PROJECT_ROOT/src/serialport/qt4support/serialport.prf)
}

TARGET = serial-mx4
TEMPLATE = app

SOURCES += main.cpp\
    qcustomplot.cpp \
    io_mx4.cpp \
    raytek_mx4.cpp \
    SerialMX4MainWindow.cpp

HEADERS  += \
    qcustomplot.h \
    io_mx4.h \
    raytek_mx4.h \
    SerialMX4MainWindow.h

FORMS    += \
    serial-mx4.ui

OTHER_FILES += \
    SerialMX4.rc

RC_FILE = SerialMX4.rc
