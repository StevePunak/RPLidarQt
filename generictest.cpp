#include "generictest.h"
#include "rangemap.h"
#include <QVector>
#include <QtGlobal>
#include <QByteArray>
#include "klog.h"

GenericTest::GenericTest()
{

}

void GenericTest::RunTest()
{
    return;
    QVector<qreal> vectors;
    for(int x = 0;x < 1440;x++)
    {
        vectors.append(x);
    }

    RangeMap rm(vectors);
    QByteArray data = rm.serialize();

    KLog::sysLogHex(data);
}
