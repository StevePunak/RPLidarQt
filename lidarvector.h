#ifndef LIDARVECTOR_H
#define LIDARVECTOR_H
#include <QtGlobal>
#include <QString>
#include "bearingandrange.h"

class LidarVector
{
public:
    LidarVector(qreal bearing, qreal range) :
        RefreshTime(0)
    {
        Vector.Bearing = bearing;
        Vector.Range = range;
    }

    BearingAndRange Vector;
    qint64 RefreshTime;

   QString ToString()
   { 
      return Vector.toString();
   }

};

#endif // LIDARVECTOR_H
