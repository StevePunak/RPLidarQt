#ifndef LIDAR_H
#define LIDAR_H

#include <QString>
#include <QSerialPort>
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
#include "lidarserver.h"
#include "lidarvector.h"
#include "lidarcommand.h"
#include "lidarresponse.h"
#include "lidarsample.h"

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE

#define LRECV_BUF_SIZE      32768

class Lidar : public QObject
{
    Q_OBJECT

public:
    Lidar(const QString& portName, qreal vectorSize);

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
private:
    QString _portName;
    QThread _thread;

    void init();

    void processReadBuffer();
    void deliverData();
    void processScanResponse(ScanResponse* handleSingleResponse);
    void processExpressScanResponse(ExpressScanResponse* handleSingleResponse);
    void trimVectors();
    int vectorArrayInc(int index);
    bool syncState();
    bool startFlagState();
    bool lengthModeAndTypeState();
    bool handleSingleResponse();
    void startSyncState();
    bool seekToByte(quint8 b);
    void removeBytesFromReceiveBuffer(int bytes);
    LidarResponse* _tryGetResponse(qint64 waitTime);
    void reset();

    void loadTestData();

private slots:
    void handleReadyRead();
    void handleTimeout();
    void handleError(QSerialPort::SerialPortError error);
    void readyWrite();
    void handleThreadStarted();

signals:
    void scanComplete(QByteArray data);
    void lidarMessage(LidarResponse& message);
    void writeData();

private:
    qreal _vectorSize;

    QSerialPort* _serialPort;

    QVector<LidarVector*> _vectors;
    QList<LidarResponse*> _responseQueue;
    quint8 _receiveBuffer[LRECV_BUF_SIZE];
    QByteArray _responseData;
    QMutex _readLock;
    QTimer* _timer;
    QByteArray _sendData;
    QWaitCondition _queueEvent;
    QWaitCondition _writeLock;
    QFile _dumpOutputFile;
    int _responseWaiters;

    qreal _offset;

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
    int _byteTotal;

    LidarServer* _server;

    int _bufferBytes;
    qint32 _bufferMsecs;
    quint64 _lastDeliveryMsecs;
};

#endif // LIDAR_H
