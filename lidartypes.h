#ifndef LIDARTYPES_H
#define LIDARTYPES_H

#include <QtGlobal>

class LidarTypes
{
public:
    enum ResponseType
    {
        DevInfo = 0x4,
        DevHealth = 0x6,
        Measurement = 0x81,
        MeasurementCapsuled = 0x82,
        MeasurementHq = 0x83,
        SampleRate = 0x15,
        MeasurementCapsuledUltra = 0x84,
        GetLidarConf = 0x20,
        SetLidarConf = 0x21,
        AccBoardFlag = 0xFF,
    };

    static inline qreal AddDegrees(qreal to, qreal count)
    {
        to += count;
        if(to >= 360)
            to -= 360;
        return to;
    }
};



#endif // LIDARTYPES_H
