#undef DEBUG_SERIAL2
#include <QDebug>
#include "lidar.h"
#include "lidarprotocol.h"
#include "blockinginterface.h"
#include "asynchinterface.h"
#include "fileinterface.h"
#include "rangemap.h"
#include "klog.h"

Lidar::Lidar(const QString& sourceName, qreal vectorSize, ReaderType readerType, quint16 listenPort, GPIO::Pin motorPin) :
    _vectorSize(vectorSize),
    _sourceName(sourceName),
    _offset(0),
    _bytesProcessed(0),
    _bytesInBuffer(0),
    _lastScanOffset(0),
    _lastGoodSampleTime(0),
    _lastBearing(0),
    _scanning(false),
    _readerType(readerType),
    _listenPort(listenPort),
    _motorPin(motorPin)
{
    for(qreal bearing = 0;bearing < 360;bearing += _vectorSize)
    {
        _vectors.append(0);
        _refreshTimes.append(0);
    }

}

void Lidar::start()
{
    _vectorRefreshTime = 500;
    _state = Sync;

    switch(_readerType)
    {
    case BlockingSerial:
        _deviceInterface = new BlockingInterface(_sourceName, _motorPin);
        break;

    case AsynchSerial:
        _deviceInterface = new AsynchInterface(_sourceName, _motorPin);
        break;

    case BinaryFile:
        _deviceInterface = new FileInterface(_sourceName, _motorPin);
        break;

    }

    connect(_deviceInterface, &DeviceInterface::deviceOpened, this, &Lidar::handleSerialPortOpened);
    connect(_deviceInterface, &DeviceInterface::dataReady, this, &Lidar::handleDataReady);
    connect(this, &Lidar::receiveDataAvailable, this, &Lidar::handleReceiveDataAvailable);

    moveToThread(&_thread);
    _thread.start();
}

void Lidar::stop()
{
    if(_deviceInterface != nullptr)
    {
        _deviceInterface->stopMotor();
    }
}

bool Lidar::getDeviceInfo()
{
    bool result = false;

    for(int tries = 20;tries > 0;--tries)
    {
        _deviceInterface->stopMotor();
        reset();

        GetDeviceInfoCommand command;
        sendCommand(command);

        LidarResponse* response = tryGetResponse(5*1000);
        if(response != nullptr)
        {
            delete response;
            result = true;
            break;
        }
    }
    return result;
}

bool Lidar::startScan()
{
    StartScanCommand command;
    _scanning = true;
    sendCommand(command);
    _deviceInterface->startMotor();
    return true;
}

bool Lidar::stopScan()
{
    StopCommand command;
    _scanning = false;
    sendCommand(command);
    _deviceInterface->stopMotor();
    return true;
}

bool Lidar::forceScan()
{
    ForceScanCommand command;
    _scanning = true;
    sendCommand(command);
    return true;
}

void Lidar::sendCommand(LidarCommand& command)
{
    _deviceInterface->send(command.Serialize());
}

void Lidar::processReadBuffer()
{
    bool completedState = true;
    do
    {
        switch(_state)
        {
        case Sync:
            if((completedState = syncState()))
            {
                _state = State::StartFlag;
            }
            break;
        case StartFlag:
            if((completedState = startFlagState()))
            {
                _state = State::LengthModeAndType;
            }
            break;
        case LengthModeAndType:
            if((completedState = lengthModeAndTypeState()))
            {
                switch(_responseMode)
                {
                case ResponseMode::SingleRequestSingleResponse:
                    _state = State::SingleResponse;
                    break;
                case ResponseMode::SingleRequestMultipleResponse:
                    _state = State::MultiResponse;
                    break;
                case ResponseMode::Reserved1:
                case ResponseMode::Reserved2:
                    startSyncState();
                    break;
                default:
                    KLog::sysLogText(KLOG_WARNING, "Illegal state: RESET");
                    reset();
                    break;
                }
            }
            break;
        case SingleResponse:
            if((completedState = handleResponse()))
            {
                LidarResponse* response = LidarResponse::Create(_responseType, reinterpret_cast<quint8*>(_responseData.data()));
                if(response != nullptr)
                {
                    if(_responseWaiters > 0)
                    {
                        _responseQueue.append(response);
                        _queueEvent.wakeAll();
                    }
                    else
                    {
                        delete response;
                    }
                }
                _state = Sync;
            }
            break;
        case MultiResponse:
            if((completedState = handleResponse()))
            {
                LidarResponse* response = LidarResponse::Create(_responseType, reinterpret_cast<quint8*>(_responseData.data()));
                if(response != nullptr)
                {
                    if(response->type() == LidarTypes::ResponseType::MeasurementCapsuled)
                    {
                        processExpressScanResponse(dynamic_cast<ExpressScanResponse*>(response));
                    }
                    else if(response->type() == LidarTypes::ResponseType::Measurement)
                    {
                        processScanResponse(dynamic_cast<ScanResponse*>(response));
                    }
                    delete response;
                }
                else
                {
                    reset();
                }
            }
            break;
        }

    } while(completedState && _bytesProcessed < _bytesInBuffer);

    int shift = _bytesProcessed;
    if(shift > 0)
    {
        removeBytesFromReceiveBuffer(shift);
    }
    _bytesProcessed = 0;
}

LidarResponse *Lidar::tryGetResponse(qint64 waitTime)
{
    _responseWaiters++;
    return _tryGetResponse(waitTime);
}

LidarResponse *Lidar::_tryGetResponse(qint64 waitTime)
{
    LidarResponse* response = nullptr;
    QMutex mutex;
    mutex.lock();
    if(_queueEvent.wait(&mutex, quint32(waitTime)))
    {
        if(_responseQueue.count() > 0)
        {
            response = _responseQueue.takeFirst();
        }
    }
    mutex.unlock();
    return response;
}

void Lidar::processExpressScanResponse(ExpressScanResponse* response)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    for(int x = 0;x < response->cabins().length();x++)
    {
        Cabin* cabin = response->cabins()[x];
        LidarSample sample1(cabin->actualAngle1(), cabin->distance1(), now);
        {
            qreal index = cabin->actualAngle1() / _vectorSize;
            _vectors[int(index)] = cabin->distance1();
            _refreshTimes[int(index)] = now;
        }
        LidarSample sample2(cabin->actualAngle2(), cabin->distance2(), now);
        {
            qreal index = cabin->actualAngle2() / _vectorSize;
            _vectors[int(index)] = cabin->distance2();
            _refreshTimes[int(index)] = now;
        }
    }
}

void Lidar::processScanResponse(ScanResponse* response)
{
    qint64 nowMsecs = QDateTime::currentMSecsSinceEpoch();
    qreal bearing = LidarTypes::AddDegrees(response->angle(), _offset);

    if(response->quality() > 10 && response->checkBit() == 1 && response->startFlag() == 1 && response->angle() < 360 && response->angle() >= 0)
    {
        qreal offset = bearing / _vectorSize;

        if(response->distance() > .010)
        {
            qreal range = qMax(response->distance(), .001);

            _vectors[int(offset)] = range;
            _refreshTimes[int(offset)] = nowMsecs;
            _lastGoodSampleTime = nowMsecs;
            if(KLog::systemVerbosity() >= 3)
                KLog::sysLogText(KLOG_DEBUG, "range: %f  bearing: %0.2f°", range, bearing);
            else if(KLog::systemVerbosity() >= 2 && bearing <= 1)
                KLog::sysLogText(KLOG_DEBUG, tr("range: %1  bearing: %2%3")
                                 .arg(range)
                                 .arg(QString().setNum(bearing, 'g', 3))
                                 .arg(QChar(0260)));
            if(bearing < _lastBearing && bearing < 10)
            {
                if(KLog::systemVerbosity() >= 3)
                    KLog::sysLogText(KLOG_INFO, "Scan complete %0.2f° < %f", bearing, _lastBearing);
                QByteArray output = RangeMap(_vectors).serialize();
                emitScanComplete(output);
            }
            _lastBearing = bearing;
            _lastScanOffset = int(offset);
        }
    }

    if(nowMsecs > _lastTrimTime + _vectorRefreshTime)
    {
        trimVectors();
    }
}

void Lidar::reset()
{
    _bytesInBuffer = _bytesProcessed = 0;
    memset(_receiveBuffer, 0, sizeof(_receiveBuffer));
    startSyncState();
}

void Lidar::emitScanComplete(const QByteArray& output)
{
    _lastScanCompletion = QDateTime::currentDateTimeUtc();
    emit scanComplete(output);
}

void Lidar::trimVectors()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    for(int x = 0;x < _vectors.length();x++)
    {
        if(now > _refreshTimes[x] + _vectorRefreshTime)
        {
            _vectors[x] = 0;
        }
    }
}

inline int Lidar::vectorArrayInc(int index)
{
    if(++index >= _vectors.length())
        index = 0;
    return index;
}

bool Lidar::syncState()
{
    return seekToByte(SYNC);
}

bool Lidar::startFlagState()
{
    return seekToByte(START_FLAG);
}

bool Lidar::seekToByte(quint8 b)
{
    bool result = false;

    int x = 0;
    for(x = _bytesProcessed;x < _bytesInBuffer;x++)
    {
        quint8 theByte = quint8(_receiveBuffer[x]);
        if(theByte == b)
            break;
    }

    if(x < _bytesInBuffer)
    {
        _bytesProcessed = x + 1;
        result = true;
    }
    else
    {
        startSyncState();
    }

    return result;
}

void Lidar::removeBytesFromReceiveBuffer(int bytes)
{
    int bytesToRemove = bytes > _bytesInBuffer ? _bytesInBuffer : bytes;
    memmove(_receiveBuffer, _receiveBuffer + bytesToRemove, _bytesInBuffer - bytesToRemove);
    _bytesInBuffer -= bytesToRemove;
}

bool Lidar::lengthModeAndTypeState()
{
    bool result = false;
    if(_bytesInBuffer - _bytesProcessed >= 5)
    {
        qint32 value;
        memcpy(&value, _receiveBuffer + _bytesProcessed, 4);
        _chunkLength = value & 0x3FFF;
        _responseMode = ResponseMode(value >> 30);
        _responseType = LidarTypes::ResponseType(_receiveBuffer[_bytesProcessed + 4]);

        _bytesProcessed += 5;
        result = true;
    }
    return result;
}

bool Lidar::handleResponse()
{
    bool result = false;
    if(_bytesInBuffer - _bytesProcessed >= _chunkLength)
    {
        _responseData.clear();
        _responseData.append(reinterpret_cast<const char*>(_receiveBuffer) + _bytesProcessed, _chunkLength);
        _bytesProcessed += _chunkLength;
        result = true;
    }
    return result;
}

void Lidar::startSyncState()
{
    _bytesProcessed = 0;
    _state = Sync;
}

void Lidar::handleDataReady(QDateTime timestamp, QByteArray data)
{
    Q_UNUSED(timestamp);

    _recvBufferLock.lock();
    int bytesToAppend = qMin(int(data.length()), int(sizeof(_receiveBuffer) - _bytesInBuffer));
    memcpy(_receiveBuffer + _bytesInBuffer, data.constData(), bytesToAppend);
    _bytesInBuffer += bytesToAppend;
    _recvBufferLock.unlock();
    emit receiveDataAvailable();
}

void Lidar::handleSerialPortOpened()
{
    emit serialPortOpened();
}

void Lidar::handleReceiveDataAvailable()
{
    _recvBufferLock.lock();
    processReadBuffer();
    _recvBufferLock.unlock();
}

void Lidar::loadTestData()
{
    QFile file;
    file.setFileName("/home/spunak/tmp/qt_lidar.bin");
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();

    int bytesToAppend = qMin(int(data.length()), int(sizeof(_receiveBuffer) - _bytesInBuffer));
    memcpy(_receiveBuffer + _bytesInBuffer, data.constData(), bytesToAppend);
    _bytesInBuffer += bytesToAppend;
    emit receiveDataAvailable();
}

