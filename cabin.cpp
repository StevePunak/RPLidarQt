#include "cabin.h"
#include "lidartypes.h"

Cabin::Cabin(qreal startAngle, quint8* frame)
{
    memcpy(_bytes, frame, sizeof(_bytes));

    _distance1 = qreal(quint16(((frame[0] >> 2) | frame[1] << 8))) / 1000;
    _distance2 = qreal(quint16(((frame[2] >> 2) | frame[3] << 8))) / 1000;
    _offsetAngle1 = (frame[0] & 0x03) | ((frame[4] & 0x08) << 2);
    _offsetAngle2 = (frame[2] & 0x03) | ((frame[4] & 0x80) >> 2);

    int chunk_0_0 = frame[0] & 0x1;
    int chunk_0_1 = frame[4] & 0xf;
    int chunk_1_0 = frame[2] & 0x1;
    int chunk_1_1 = frame[4] >> 4;
    int q3_1 = (chunk_0_1 << 1) | chunk_0_0;
    int q3_2 = (chunk_1_1 << 1) | chunk_1_0;

    _offsetAngle1 = qreal(q3_1) / 8.0;
    _actualAngle1 = LidarTypes::AddDegrees(_offsetAngle1, startAngle);
    _offsetAngle2 = qreal(q3_2) / 8.0;
    _actualAngle2 = LidarTypes::AddDegrees(_offsetAngle2, startAngle);
}

QString Cabin::ToString()
{
    QString ret;
    ret.sprintf("Distance 1: %f  Distance 2: %f  Angle 1: %f  Angle 2: %f", _distance1, _distance2, _actualAngle1, _actualAngle2);
    return ret;
}
