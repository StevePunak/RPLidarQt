#ifndef RANGEMAP_H
#define RANGEMAP_H
#include <QVector>
#include <QByteArray>
#include <QDataStream>
#include <QFile>

class RangeMap
{
public:
    RangeMap(QVector<qreal> vectors);

    QByteArray serialize();

    static const QByteArray StartMarker;

private:
    QVector<qreal> _vectors;
    static QFile _outputFile;
    static QDataStream _outputStream;
    static bool _outputToFile;
};

#endif // RANGEMAP_H
