#ifndef CABIN_H
#define CABIN_H

#include <QtGlobal>
#include <QByteArray>
#include <QString>

class Cabin
{
public:
    Cabin(qreal startAngle, quint8* frame);
    qreal distance1() const { return _distance1; }
    qreal distance2() const { return _distance2; }
    qreal OffsetAngle1() const { return _offsetAngle1; }
    qreal OffsetAngle2() const { return _offsetAngle2; }
    qreal actualAngle1() const { return _actualAngle1; }
    qreal actualAngle2() const { return _actualAngle2; }

    quint8* Bytes() { return _bytes; }
    QString ToString();

private:
    qreal _distance1;
    qreal _distance2;
    qreal _offsetAngle1;
    qreal _offsetAngle2;
    qreal _actualAngle1;
    qreal _actualAngle2;

    quint8 _bytes[5];
};

#endif // CABIN_H
