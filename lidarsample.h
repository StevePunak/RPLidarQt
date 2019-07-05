#ifndef LIDARSAMPLE_H
#define LIDARSAMPLE_H

#include <QtGlobal>
#include <QDateTime>
#include <QString>
#include "bearingandrange.h"


class LidarSample
{
public:
    LidarSample(qreal bearing, qreal range, qint64& timestamp) :
        TimeStamp(timestamp)
    {
        Vector.Bearing = bearing;
        Vector.Range = range;
    }

    BearingAndRange Vector;
    qint64 TimeStamp;

    QString ToString();
};

#endif // LIDARSAMPLE_H
