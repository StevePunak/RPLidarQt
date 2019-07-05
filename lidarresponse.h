#ifndef LIDARRESPONSE_H
#define LIDARRESPONSE_H
#include <QString>
#include <QtGlobal>
#include <QVector>

#include "lidartypes.h"
#include "cabin.h"

class LidarResponse
{
protected:
    LidarResponse(LidarTypes::ResponseType type) :
        _type(type) {}

public:
    virtual ~LidarResponse();

    static LidarResponse* Create(LidarTypes::ResponseType responseType, quint8* responseBytes);

    LidarTypes::ResponseType type() const { return _type; }
private:

    LidarTypes::ResponseType _type;
};

class DeviceInfoResponse : public LidarResponse
{
public:
    DeviceInfoResponse(quint8* responseBytes);

    QString ToString();

    quint8 Model() const { return _model; }
    quint8 FirmwareVersionMajor() const { return _firmwareVersionMajor; }
    quint8 FirmwareVersionMinor() const { return _firmwareVersionMinor; }
    quint8 Hardware() const {return _hardware; }
    QString SerialNumber() const { return _serialNumber; }

private:
    quint8 _model;
    quint8 _firmwareVersionMinor;
    quint8 _firmwareVersionMajor;
    quint8 _hardware;
    QString _serialNumber;
};


class GetSampleRateResponse : public LidarResponse
{
public:
    GetSampleRateResponse(quint8* responseBytes);

    int Standard() const { return _standard; }
    int Express() const { return _express; }

    QString ToString();

private:
    int _standard;
    int _express;

};

class ExpressScanResponse : public LidarResponse
{
public:
    ExpressScanResponse(quint8* responseBytes);
    virtual ~ExpressScanResponse();

    quint8 Sync() const { return _sync; }
    quint8 Checksum() const { return _checksum; }
    qreal StartAngle() const { return _startAngle; }
    bool NewScan() const { return _newScan; }

    QVector<Cabin*>& cabins() { return _cabins; }

    QByteArray& Bytes() { return _bytes; }
    QString ToString();

private:
    quint8 _sync;
    quint8 _checksum;
    qreal _startAngle;
    bool _newScan;

    QByteArray _bytes;
    QVector<Cabin*> _cabins;
};


class ScanResponse : public LidarResponse
{
public:
    ScanResponse(quint8* responseBytes);

    int quality() const { return _quality; }
    int startFlag() const { return _startFlag; }
    int checkBit() const { return _checkBit; }
    int AngleQ6() const { return _angleQ6; }
    qreal angle() const { return _angle; }
    int DistanceQ2() const { return _distanceQ2; }
    qreal distance() const { return _distance; }

    quint8* Bytes() { return _bytes; }

    QString ToString();

private:
    int _quality;
    int _startFlag;
    int _checkBit;
    int _angleQ6;
    qreal _angle;
    int _distanceQ2;
    qreal _distance;

    quint8 _bytes[5];

};

#endif // LIDARRESPONSE_H
