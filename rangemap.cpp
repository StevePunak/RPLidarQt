#include "rangemap.h"
#include <QDataStream>
#include <QDateTime>
#include "klog.h"

const QByteArray RangeMap::StartMarker = QString("!!RP!!").toLocal8Bit();

QFile RangeMap::_outputFile;
QDataStream RangeMap::_outputStream;
bool RangeMap::_outputToFile = false;

// format of serialized rangemap is
// !!RP!!{16-bit count}{uint64 milliseconds since epoch}{count * sizeof(qreal)}
// First entry is range at 0 degrees up through range at number of 360 / number of vectors
// in rangemap
RangeMap::RangeMap(QVector<qreal> vectors) :
    _vectors(vectors)
{
    if(_outputToFile && _outputFile.isOpen() == false)
    {
        QString fileName = "/tmp/lidar_out.bin";
        if(_outputFile.exists())
        {
            QFile::remove(fileName);
        }
        _outputFile.setFileName(fileName);
        _outputFile.open(QIODevice::WriteOnly);
        _outputStream.setDevice(&_outputFile);
    }
}

QByteArray RangeMap::serialize()
{
    QByteArray serialized;
    serialized.reserve(StartMarker.length() + sizeof(quint16) + sizeof(quint16) + (sizeof(qreal) * _vectors.length()));

    QDataStream output(&serialized, QIODevice::WriteOnly);
    output.writeRawData(StartMarker.constData(), StartMarker.length());
    output << (quint16)_vectors.length();
    output << (quint64)QDateTime::currentMSecsSinceEpoch();
    foreach(qreal range, _vectors)
    {
        output << range;
    }

    if(_outputToFile)
    {
        _outputStream.writeRawData(serialized, serialized.length());
        _outputFile.flush();
    }
//    KLog::sysLogHex(serialized);
    return serialized;
}
