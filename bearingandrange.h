#ifndef BEARINGANDRANGE_H
#define BEARINGANDRANGE_H

#include <QtGlobal>
#include <QDateTime>
#include <QString>

class BearingAndRange
{
public:
    BearingAndRange() :
        Bearing(0),
        Range(0) {}

    qreal Bearing;
    qreal Range;

    QString toString()
    {
        return QString().sprintf("Bearing: %f°  Range: %f", Bearing, Range);
    }
};

#endif // BEARINGANDRANGE_H
