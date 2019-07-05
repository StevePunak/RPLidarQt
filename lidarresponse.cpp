#include "lidarresponse.h"
#include <QDebug>
#include <QDataStream>
#include "klog.h"

LidarResponse::~LidarResponse()
{

}

LidarResponse *LidarResponse::Create(LidarTypes::ResponseType responseType, quint8 *responseBytes)
{
    LidarResponse* response = nullptr;

    switch(responseType)
    {
        case LidarTypes::ResponseType::DevInfo:
            response = new DeviceInfoResponse(responseBytes);
            break;
        case LidarTypes::ResponseType::MeasurementCapsuled:
            response = new ExpressScanResponse(responseBytes);
            break;
        case LidarTypes::ResponseType::SampleRate:
            response = new GetSampleRateResponse(responseBytes);
            break;
        case LidarTypes::ResponseType::Measurement:
            response = new ScanResponse(responseBytes);
            break;
        case LidarTypes::ResponseType::DevHealth:
        case LidarTypes::ResponseType::MeasurementHq:
        case LidarTypes::ResponseType::MeasurementCapsuledUltra:
        case LidarTypes::ResponseType::GetLidarConf:
        case LidarTypes::ResponseType::SetLidarConf:
        case LidarTypes::ResponseType::AccBoardFlag:
            qDebug() << "Unsupported type to factory" << responseType << endl;
            break;
    }
    return response;

}

DeviceInfoResponse::DeviceInfoResponse(quint8 *responseBytes) :
    LidarResponse(LidarTypes::ResponseType::DevInfo)
{
    _model = responseBytes[0];
    _firmwareVersionMinor = responseBytes[1];
    _firmwareVersionMajor = responseBytes[2];
    _hardware = responseBytes[3];
    _serialNumber = "";

}

ScanResponse::ScanResponse(quint8* responseBytes) :
    LidarResponse(LidarTypes::ResponseType::Measurement)
{
    _quality = responseBytes[0] >> 2;
    _startFlag = (responseBytes[0] >> 1) & 0x01;
    _checkBit = responseBytes[1] & 0x01;
    _angleQ6 = (responseBytes[2] << 7) | (responseBytes[1] >> 1);
    _angle = qreal(_angleQ6) / 64.0;
    _distance = qreal((((responseBytes[4] << 8) | responseBytes[3])) / 4.0) / 1000;
}

QString ScanResponse::ToString()
{
    QString ret;
    ret.sprintf("Q: %d  Angle: %f  Distance: %f   sf: %d  cb: %d",
        _quality, _angle, _distance, _startFlag, _checkBit);
    return ret;
}

ExpressScanResponse::ExpressScanResponse(quint8* responseresponseBytes) :
    LidarResponse(LidarTypes::ResponseType::MeasurementCapsuled)
{
    quint8 s0 = responseresponseBytes[0] >> 4;
    quint8 s1 = responseresponseBytes[1] >> 4;
    _sync = quint8((s0 << 4) | s1);

    quint8 c0 = responseresponseBytes[0] & 0x0f;
    quint8 c1 = responseresponseBytes[1] & 0x0f;
    _checksum = quint8(((c0 << 4) | c1));

    qreal q6Angle = ((responseresponseBytes[3] << 8) | responseresponseBytes[2]) & 0x7fff;
    _startAngle = q6Angle / 64;

    _newScan = (responseresponseBytes[3] & 0x80) != 0 ? true : false;

    quint8* ptr = responseresponseBytes + 4;
    for(int x = 0;x < 16;x++, ptr += 5)
    {
        Cabin* cabin = new Cabin(_startAngle, ptr);
        _cabins.append(cabin);
    }
}

ExpressScanResponse::~ExpressScanResponse()
{
    for(int x = 0;x < _cabins.length();x++)
    {
        delete _cabins[x];
    }
}


GetSampleRateResponse::GetSampleRateResponse(quint8 *responseBytes) :
    LidarResponse(LidarTypes::ResponseType::SampleRate)
{
    _standard = *reinterpret_cast<qint16*>(responseBytes);
    _express = *reinterpret_cast<qint16*>(responseBytes + 2);
}
