#include "lidarsample.h"


QString LidarSample::ToString()
{
    QString output;
    output.sprintf("Sample %f %f", Vector.Bearing, Vector.Range);
    return output;
}
