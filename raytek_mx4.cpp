#include "raytek_mx4.h"

RaytekMX4::RaytekMX4() :
    SerialX( new XCHANGE_THREAD/*(this)*/ ),
    SerialBM( new BM_THREAD/*(this)*/ )
{
    map[ReadDeviceBurnID] = "DI";
    map[UserInterfaceLock] =  "UI";
    /* One character commands (for faster bursts): */
    map[Time]           = "@";
    map[Celsius]        = "C";
    map[Date]           = "D";
    map[Emissivity]     = "E";
    map[Fahrenheit]     = "F";
    map[AverageTemp]    = "G";
    map[HighestTemp]    = "H";
    map[InternalTemp]   = "I";
    map[LowestTemp]     = "L";
    map[Power]          = "P";
    map[Status]         = "S";
    map[TargetTemp]     = "T";
    map[DeviceUnit]     = "U";
    map[TempOfExternProbe] = "X";
    map[BurstMode]      = "BM";

    ctrl[ask] = "?";
    ctrl[set] = "=";
    ctrl[ack] = "!";
    ctrl[err] = "*";
    ctrl[def] = "$";
    ctrl[CR] = "\r\n";
    ctrl[ask] = "\x1B";

    BURST_MODE = false;
    UI_LOCKED = false;
    SETABLE = false;

    connect( SerialX,   SIGNAL(otvet(QString)),     SIGNAL(otvet_mx4(QString)) );
    connect( SerialX,   SIGNAL(error(QString)),     SIGNAL(error_mx4(QString)) );
    connect( SerialX,   SIGNAL(timeout(QString)),   SIGNAL(timeout_mx4(QString)) );
    connect( SerialBM,  SIGNAL(otvet(QString)),     SIGNAL(otvet_mx4(QString)) );
    connect( SerialBM,  SIGNAL(error(QString)),     SIGNAL(error_mx4(QString)) );
    connect( SerialBM,  SIGNAL(timeout(QString)),   SIGNAL(timeout_mx4(QString)) );
}

RaytekMX4::~RaytekMX4()
{
    delete SerialBM;
    delete SerialX;
}

QString RaytekMX4::formSet(eRequest Param,bool value)
{
    QString V = value ? "1" : "0";
    return map[Param] + ctrl[set] + V + ctrl[CR];
}

QString RaytekMX4::formAsk(eRequest Param)
{
    return QString(ctrl[ask] + map[Param]);
}

QString RaytekMX4::parse(const QString& s)
{
    QString answer(s);

//    if( answer.startsWith("!") )
//    {
//        answer.remove(0,1);                         // delete "!"
//        answer.remove(0,map[request].length()+1);   // delete request code (T,UI,DI,...) and "="
//        if( answer.endsWith("\r\n")) {
//            answer.remove(answer.length()-2,2);
//        }
//    }
//    else {
//        return "Error: " + answer +"\n";
//    }
    return answer;
}

bool RaytekMX4::BurstRunning()
{
    return BURST_MODE;
}

QString RaytekMX4::raw_answer(const QString& s)
{
    return QString(s);
}

void RaytekMX4::AskTemp()
{
    request = formAsk(TargetTemp);            // ?T
    SerialX->zapros(request);
}

void RaytekMX4::SetUserInterfaceOn()
{
    request = formSet(UserInterfaceLock,1);   // UI=1\r\n
    SerialX->zapros(request);
}

void RaytekMX4::SetUserInterfaceOff()
{
    request = formSet(UserInterfaceLock,0);   // UI=0\r\n
    SerialX->zapros(request);
}

void RaytekMX4::SetBurstString()              // define the burst string
{
    request = ctrl[def] + ctrl[set] + map[TargetTemp] + map[TempOfExternProbe] + ctrl[CR];   // $=TX\r\n
    SerialX->zapros(request);
}

void RaytekMX4::StartBurstMode()
{
    if( SerialX->isRunning() )
    {
        SerialX->stop_otkl();
    }
    //request = formSet(BurstMode,1);   // BM=1\r\n
    BURST_MODE = true;
    SerialBM->zapusk();
}

void RaytekMX4::StopBurstMode()
{
    //request = formSet(BurstMode,0);   // BM=0\r\n
    //request = "ESC";   // BM=0\r\n
    //SerialX->zapros(request);
    if( SerialBM->isRunning() )
    {
        SerialBM->stop_otkl();
    }
}
