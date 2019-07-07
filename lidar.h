#ifndef LIDAR_H
#define LIDAR_H

#include <QString>
#include <QVector>
#include <QtGlobal>
#include <QTimer>
#include <QMutex>
#include <QByteArray>
#include <QDateTime>
#include <QTime>
#include <QThread>
#include <QWaitCondition>
#include <QList>
#include <QFile>
#include "deviceinterface.h"
#include "lidarserver.h"
#include "lidarvector.h"
#include "lidarcommand.h"
#include "lidarresponse.h"
#include "lidarsample.h"
#include "gpio.h"

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE


class Lidar : public QObject
{
    Q_OBJECT

public:
    enum ReaderType { BlockingSerial, AsynchSerial, BinaryFile };

    Lidar(const QString& portName, qreal vectorSize, ReaderType type, quint16 listenPort, GPIO::Pin motorPin);

    enum State
    {
        Sync,
        StartFlag,
        LengthModeAndType,
        SingleResponse,
        MultiResponse
    };

    enum ResponseMode
    {
        SingleRequestSingleResponse = 0x0,
        SingleRequestMultipleResponse = 0x01,
        Reserved1 = 0x02,
        Reserved2 = 0x03,
    };

    bool GetDeviceInfo();
    bool StartScan();

    void SendCommand(LidarCommand& command);
    LidarResponse* TryGetResponse(qint64 waitTime);

    bool ForceScan();
    bool StopScan();

private:
    void init();

    void processReadBuffer();
    void deliverData();
    void processScanResponse(ScanResponse* handleResponse);
    void processExpressScanResponse(ExpressScanResponse* handleResponse);
    void trimVectors();
    int vectorArrayInc(int index);
    bool syncState();
    bool startFlagState();
    bool lengthModeAndTypeState();
    bool handleResponse();
    void startSyncState();
    bool seekToByte(quint8 b);
    void removeBytesFromReceiveBuffer(int bytes);
    LidarResponse* _tryGetResponse(qint64 waitTime);
    void reset();

    void loadTestData();

private slots:
    void handleDataReady(QByteArray data);

signals:
    void scanComplete(QByteArray rangeData);

private:
    QThread _thread;

    qreal _vectorSize;

    QString _sourceName;
    QVector<qreal> _vectors;
    QVector<qint64> _refreshTimes;
    QList<LidarResponse*> _responseQueue;
    QByteArray _responseData;
    QMutex _readLock;
    QByteArray _sendData;
    QWaitCondition _queueEvent;
    QWaitCondition _writeLock;
    QFile _dumpOutputFile;
    int _responseWaiters;

    qreal _offset;

    quint8 _receiveBuffer[32786];
    int _bytesProcessed;
    int _bytesInBuffer;

    State _state;
    ResponseMode _responseMode;
    LidarTypes::ResponseType _responseType;

    int _lastScanOffset;

    qint64 _lastTrimTime;
    qint64  _lastGoodSampleTime;
    qint64 _vectorRefreshTime;
    qreal _lastBearing;

    qint32 _chunkLength;
    bool _scanning;

    LidarServer* _server;
    DeviceInterface* _reader;

    quint16 _listenPort;
    GPIO::Pin _motorPin;
};

#endif // LIDAR_H
