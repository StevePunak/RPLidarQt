#include "lidarcommand.h"

#include <QDataStream>

LidarCommand::LidarCommand(LidarCommand::CommandCode commandCode)
{
 _code = commandCode;
}

LidarCommand::LidarCommand(LidarCommand::CommandCode commandCode, const quint8* payload, int length)
{
    _code = commandCode;
    _payload.append(reinterpret_cast<const char*>(payload), length);
}

QByteArray& LidarCommand::Serialize()
{
    qint32 length = _payload.length() > 0 ? HEADER_SIZE + SIZE_SIZE + _payload.length()+ CHECKSUM_SIZE
                                          : HEADER_SIZE;
    quint8 cs = 0;

    QDataStream output(&_serialized, QIODevice::WriteOnly);
    output << Marker;
    cs ^= Marker;
    output << quint8(_code);
    cs ^= quint8(_code);

    if(length > HEADER_SIZE)
    {
        qint32 payloadLength = _payload.length();

        output << quint8(payloadLength);
        cs ^= quint8(payloadLength);

        output << _payload;
        const char* start = _payload.constData();
        for(const char* a = start;a < start + payloadLength;a++)
        {
            cs ^= *a;
        }
    }
    return _serialized;
}


SetMotorPwm::SetMotorPwm(quint16 pwm) :
    LidarCommand(CommandCode::SetMotorPwm)
{
    QDataStream output(&_payload, QIODevice::WriteOnly);
    output << pwm;
}

ResetCommand::ResetCommand() :
    LidarCommand(CommandCode::Reset)
{
}

StopCommand::StopCommand() :
    LidarCommand(CommandCode::Stop)
{
}

StartScanCommand::StartScanCommand() :
    LidarCommand(CommandCode::Scan)
{
}

ForceScanCommand::ForceScanCommand() :
    LidarCommand(CommandCode::ForceScan)
{
}

const quint8 StartExpressScanCommand::PAYLOAD[5] = { 0, 0, 0, 0, 0 };
StartExpressScanCommand::StartExpressScanCommand() :
    LidarCommand(CommandCode::ExpressScan, PAYLOAD, sizeof(PAYLOAD))
{

}

GetSampleRateCommand::GetSampleRateCommand() :
    LidarCommand(CommandCode::GetSampleRate)
{

}

const quint8 GetAccBoardCommand::PAYLOAD[4] = { 0, 0, 0, 0 };
GetAccBoardCommand::GetAccBoardCommand() :
    LidarCommand(CommandCode::GetAccBoardFlag, PAYLOAD, sizeof(PAYLOAD))
{

}

GetDeviceInfoCommand::GetDeviceInfoCommand() :
    LidarCommand(CommandCode::GetDeviceInfo)
{

}
