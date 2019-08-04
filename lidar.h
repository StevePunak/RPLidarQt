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
    void start();
    void stop();

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

    bool getDeviceInfo();
    bool startScan();

    void sendCommand(LidarCommand& command);
    LidarResponse* tryGetResponse(qint64 waitTime);

    bool forceScan();
    bool stopScan();

    void startMotor() { _deviceInterface->startMotor(); }
    void stopMotor() { _deviceInterface->stopMotor(); }

    bool isDeviceOpen() const { return _deviceInterface != nullptr ? _deviceInterface->deviceOpen() : false; }

    QDateTime lastScanCompletion() const {return _lastScanCompletion; }

private:
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
    void emitScanComplete(const QByteArray& output);

    void loadTestData();

private slots:
    void handleDataReady(QDateTime timestamp, QByteArray data);
    void handleSerialPortOpened();
    void handleReceiveDataAvailable();

signals:
    void scanComplete(QByteArray rangeData);
    void serialPortOpened();
    void receiveDataAvailable();

private:
    QThread _thread;

    qreal _vectorSize;

    QString _sourceName;
    QVector<qreal> _vectors;
    QVector<qint64> _refreshTimes;
    QList<LidarResponse*> _responseQueue;
    QByteArray _responseData;
    QMutex _recvBufferLock;
    QByteArray _sendData;
    QWaitCondition _queueEvent;
    QWaitCondition _writeLock;
    QFile _dumpOutputFile;
    int _responseWaiters;
    QDateTime _lastScanCompletion;

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

    DeviceInterface* _deviceInterface;
    ReaderType _readerType;

    quint16 _listenPort;
    GPIO::Pin _motorPin;
};

#endif // LIDAR_H
