#ifndef RAYTEK_MX4_H
#define RAYTEK_MX4_H

#include <QString>
#include <QMap>

#include "io_mx4.h"

class RaytekMX4 : public QObject
{
    Q_OBJECT

public:
    RaytekMX4();
    ~RaytekMX4();

    bool BurstRunning();
    QString raw_answer(const QString&);

public slots:
    void AskTemp();
    void SetBurstString();
    void StartBurstMode();
    void StopBurstMode();
    void SetUserInterfaceOn();
    void SetUserInterfaceOff();

signals:
    void otvet_mx4(const QString &s);   // Ответ от MX4 Ranger
    void error_mx4(const QString &s);   // Ошибка при открытии/настройке serial-порта
    void timeout_mx4(const QString &s); // Превышен таймаут чтения/записи serial-порта

private:
    XCHANGE_THREAD* SerialX;        // Поток для обмена сообщениями с ИК-термометром
    BM_THREAD* SerialBM;            // Поток, специализированный для чтения температуры из ИК-термометра

    enum eRequest {
       ReadDeviceBurnID, // DI
       UserInterfaceLock,// UI: "?UI", "UI=0" disables keys, switches and switch off, "UI=1" enables
       /* One character commands (for faster bursts): */
       Time,             // @: ?@ or "@=11:59:10AM"
       Celsius,          // C: only in Burststr: "C T238.1"
       Date,             // D: ?D or "Date=98/03/12"
       Emissivity,       // E: ?E or "E=.9"
       Fahrenheit,       // F: only in Burststr: "F T460.6"
       AverageTemp,      // G: ?G->"G238.1" ,moving average, 2s to 90%
       HighestTemp,      // H: (Max/Peak) ?H, Maximum since start of measurement
       InternalTemp,     // I: "?I", internal temperature (heatsink)
       LowestTemp,       // L: (Min/Valley)	?L, Minimum since start of measurement
       Power,            // P: "?P", device specific energy value
       Status,           // S: (Target T./Internal T. out of Limits) "?S", (two digits for out-of-limits-bits and alert-bits)
       TargetTemp,       // T: ?T, Object temperature
       DeviceUnit,       // U: ?U, "U=F" or "U=C"
       TempOfExternProbe,// X: TC-J, TC-K, NTC Temp. ?X, Temperature of external probe
       BurstMode,        // BM (on/off)
    };
    /* Control character */
    enum eControl {
        ask,            // ?    Ask parameter value
        set,            // =    Set parameter value
        ack,            // !    Acknowledge parameter change
        err,            // *    Error string (Not acknowledged)
        def,            // $    Define or request burst output string format
        CR,             // CR 	End of input command
        ESC             // ESC	Abort Burst or Stream actions
    };
    QString formSet(eRequest,bool);
    QString formAsk(eRequest);
    QString parse(const QString&);

    QMap<eRequest,QString> map;
    QMap<eControl,QString> ctrl;

    bool BURST_MODE;      // BM=1 было отправлено на термометр
    bool UI_LOCKED;       // UI=0 было отправлено на термометр
    bool SETABLE;         // =0-ask, =1-set
    QString request;      // eRequest type
};

#endif // RAYTEK_MX4_H
