#ifndef LIDARCOMMAND_H
#define LIDARCOMMAND_H

#include <QtGlobal>
#include <QByteArray>

const quint8 Marker = 0xa5;

const qint32 HEADER_SIZE = 2;
const quint8 HAS_PAYLOAD = 0x80;
const qint32 SIZE_SIZE = 1;
const qint32 CHECKSUM_SIZE = 1;

class LidarCommand
{
public:
    enum CommandCode
    {
    // Commands without payload and response
        Stop = 0x25,
        Scan = 0x20,
        ForceScan = 0x21,
        Reset = 0x40,

    // Commands without payload but have response
        GetDeviceInfo = 0x50,
        GetDeviceHealth = 0x52,
        GetSampleRate = 0x59, //added in fw 1.17,
        HqMotorSpeedCtrl = 0xA8,

    // Commands with payload and have response
        ExpressScan = 0x82, //added in fw 1.17,
        HqScan = 0x83, //added in fw 1.24,
        GetLidarConf = 0x84, //added in fw 1.24,
        SetLidarConf = 0x85, //added in fw 1.24,

    //add for A2 to set RPLIDAR motor pwm when using accessory board
        SetMotorPwm = 0xF0,
        GetAccBoardFlag = 0xFF,
    };

    LidarCommand(CommandCode commandCode);
    LidarCommand(CommandCode commandCode, const quint8* payload, int size);

    QByteArray& Serialize();

    CommandCode Code() const { return _code; }
    QByteArray& Payload() { return _payload; }
    quint8 Checksum() const { return _checksum; }
    bool HasPayload() const { return _payload.count() > 0; }

protected:
    CommandCode _code;
    QByteArray _payload;
    QByteArray _serialized;
    quint8 _checksum;
};

class SetMotorPwm : public LidarCommand
{
public:
    SetMotorPwm(quint16 pwm);
};

class ResetCommand : public LidarCommand
{
public:
    ResetCommand();
};

class StopCommand : public LidarCommand
{
public:
    StopCommand();
};

class ForceScanCommand : public LidarCommand
{
public:
    ForceScanCommand();
};

class StartScanCommand : public LidarCommand
{
public:
    StartScanCommand();
};

class StartExpressScanCommand : public LidarCommand
{
public:
    StartExpressScanCommand();

private:
    static const quint8 PAYLOAD[5];
};

class GetSampleRateCommand : public LidarCommand
{
public:
    GetSampleRateCommand();
};

class GetAccBoardCommand : public LidarCommand
{
public:
    GetAccBoardCommand();
private:
    static const quint8 PAYLOAD[4];
};

class GetDeviceInfoCommand : public LidarCommand
{
public:
    GetDeviceInfoCommand();
};



#endif // LIDARCOMMAND_H
