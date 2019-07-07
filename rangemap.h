#ifndef RANGEMAP_H
#define RANGEMAP_H
#include <QVector>
#include <QByteArray>

class RangeMap
{
public:
    RangeMap(QVector<qreal> vectors) :
        _vectors(vectors) {}

    QByteArray serialize();

    static const QByteArray StartMarker;

private:
    QVector<qreal> _vectors;
};

#endif // RANGEMAP_H
