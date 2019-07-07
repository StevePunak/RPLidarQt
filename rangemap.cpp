#include "rangemap.h"
#include <QDataStream>

const QByteArray RangeMap::StartMarker("!!RP!!");

// format of serialized rangemap is
// !!RP!!{16-bit count}{count * sizeof(qreal)}
// First entry is range at 0 degrees up through range at number of 360 / number of vectors
// in rangemap
QByteArray RangeMap::serialize()
{
    QByteArray serialized;
    serialized.reserve(StartMarker.length() + sizeof(quint16) + (sizeof(qreal) * _vectors.length()));

    QDataStream output(&serialized, QIODevice::WriteOnly);
    output << StartMarker;
    output << (quint16)_vectors.length();
    foreach(qreal i, _vectors)
    {
        output << _vectors[i];
    }
    return serialized;
}
