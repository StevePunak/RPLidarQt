#undef DEBUG_SERIAL2
#include "lidar.h"
#include "lidarprotocol.h"
#include <QDebug>
#include "klog.h"

Lidar::Lidar(const QString& portName, qreal _vectorSize) :
    _portName(portName),
    _vectorSize(_vectorSize),
    _offset(0),
    _bytesProcessed(0),
    _bytesInBuffer(0),
    _lastScanOffset(0),
    _lastGoodSampleTime(0),
    _lastBearing(0),
    _scanning(false),
    _byteTotal(0),
    _server(nullptr),
    _bufferBytes(4096),
    _bufferMsecs(5000),
    _lastDeliveryMsecs(0)
{
    for(qreal bearing = 0;bearing < 360;bearing += _vectorSize)
    {
        LidarVector* vector = new LidarVector(bearing, 0);
        _vectors.append(vector);
    }

    _vectorRefreshTime = 500;
    _state = Sync;

    //loadTestData();

    init();
}

void Lidar::init()
{
    _timer = new QTimer();

    _serialPort = new QSerialPort("Lidar Read");
    _serialPort->setPortName(_portName);

    _serialPort->setBaudRate(QSerialPort::Baud115200);

    if(_serialPort->open(QIODevice::ReadWrite) == false)
    {
        QSerialPort::SerialPortError error = _serialPort->error();
        qDebug() << "Error... " << error;
    }

    _server = new LidarServer(this, this);

    connect(this, &Lidar::scanComplete, _server, &LidarServer::handleScanReady);
    connect(_serialPort, &QSerialPort::readyRead, this, &Lidar::handleReadyRead);
    connect(_serialPort, &QSerialPort::errorOccurred, this, &Lidar::handleError);
    connect(_timer, &QTimer::timeout, this, &Lidar::handleTimeout);
    connect(this, &Lidar::writeData, this, &Lidar::readyWrite, Qt::ConnectionType::QueuedConnection);

    connect(&_thread, &QThread::started, this, &Lidar::handleThreadStarted);
    moveToThread(&_thread);

    _serialPort->moveToThread(&_thread);
    _timer->moveToThread(&_thread);
    _thread.start();

#ifdef DEBUG_SERIAL2
    _dumpOutputFile.setFileName("/home/pi/tmp/lidar.bin");
    if(!_dumpOutputFile.open(QIODevice::WriteOnly))
    {
        KLog::sysLogText(KLOG_ERROR, "Could not open file");
    }
    _dumpOutputFile.moveToThread(&_thread);
#endif
}

bool Lidar::GetDeviceInfo()
{
    qDebug() << "Clear Start";

    QWaitCondition wait;
    QMutex mutex;
    mutex.lock();
    wait.wait(&mutex, 5000);
    mutex.unlock();

    qDebug() << "Clear End";

    GetDeviceInfoCommand command;
    SendCommand(command);

    LidarResponse* response = TryGetResponse(20000);
    if(response != nullptr)
    {
        qDebug() << "Got device info";
        delete response;
    }
    else
    {
        qDebug() << "NO device info";
    }
    return response != nullptr;
}

bool Lidar::StartScan()
{
    StartScanCommand command;
    qDebug() << "Start Scan";
    _scanning = true;
    SendCommand(command);
    return true;
}

bool Lidar::ForceScan()
{
    ForceScanCommand command;
    qDebug() << "Force Scan";
    _scanning = true;
    SendCommand(command);
    return true;
}

void Lidar::SendCommand(LidarCommand& command)
{
    _sendData = command.Serialize();
    emit writeData();
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
            qDebug() << "Start Flag State";
            if((completedState = startFlagState()))
            {
                _state = State::LengthModeAndType;
            }
            break;
        case LengthModeAndType:
            qDebug() << "Length Mode Type State";
            if((completedState = lengthModeAndTypeState()))
            {
#ifdef DEBUG_SERIAL
                Log.SysLogText(LogLevel.DEBUG, "Response mode ===>>> {0}", _responseMode);
#endif
                qDebug() << "Length Mode Type State completed";
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
            qDebug() << "Single Response State";
            if((completedState = handleSingleResponse()))
            {
                qDebug() << "Single Response State completed";
#ifdef DEBUG_SERIAL
                Log.SysLogText(LogLevel.DEBUG, "RECEVED COMPLETE SINGLE RESPONSE");
#endif
                LidarResponse* response = LidarResponse::Create(_responseType, reinterpret_cast<quint8*>(_responseData.data()));
                if(response != nullptr)
                {
                    lidarMessage(*response);
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
#ifdef DEBUG_SERIAL
            Log.SysLogText(LogLevel.DEBUG, "State ===>>> {0} have {1} bytes  offset {2}", _state, _bytesInBuffer, _recvOffset);
#endif
            if((completedState = handleSingleResponse()))
            {
#ifdef DEBUG_SERIAL
                    Log.SysLogText(LogLevel.DEBUG, "RECEVED COMPLETE MULTI RESPONSE now have {0} bytes  offset {1}", _bytesInBuffer, _recvOffset);
                    Log.SysLogHex(LogLevel.DEBUG, _responseData);
#endif
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
                    lidarMessage(*response);
                    delete response;
                }
                else
                {
                    startSyncState();
                }
            }
            break;
        }

#ifdef DEBUG_SERIAL2
        KLog::sysLogText(KLOG_DEBUG, "Completed: %d  offset: 0x%x   bytes: 0x%x  state: %d",
                          completedState, _bytesProcessed, _bytesInBuffer, _state);
#endif
    } while(completedState && _bytesProcessed < _bytesInBuffer);

#ifdef DEBUG_SERIAL
    Log.SysLogText(LogLevel.DEBUG, "Now in State {0}, offset {1} there are {2} bytes left", _state, _recvOffset, _bytesInBuffer);
#endif
    int shift = _bytesProcessed;
    if(shift > 0)
    {
        removeBytesFromReceiveBuffer(shift);
    }
    _bytesProcessed = 0;

}

LidarResponse *Lidar::TryGetResponse(qint64 waitTime)
{
    _responseWaiters++;
    return _tryGetResponse(waitTime);
}

LidarResponse *Lidar::_tryGetResponse(qint64 waitTime)
{
    qDebug() << "Begin wait on thread " << QThread::currentThreadId();
    LidarResponse* response = nullptr;
    QMutex mutex;
    mutex.lock();
    if(_queueEvent.wait(&mutex, quint32(waitTime)))
    {
        //_readLock.lock();
        if(_responseQueue.count() > 0)
        {
            response = _responseQueue.takeFirst();
        }
        //_readLock.unlock();
    }
    mutex.unlock();
    qDebug() << "End wait with " << ((response == nullptr) ? "No repsonse" : "A good response");
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
            _vectors[int(index)]->Vector.Range = cabin->distance1();
            _vectors[int(index)]->RefreshTime = now;
        }
        LidarSample sample2(cabin->actualAngle2(), cabin->distance2(), now);
        {
            qreal index = cabin->actualAngle2() / _vectorSize;
            _vectors[int(index)]->Vector.Range = cabin->distance2();
            _vectors[int(index)]->RefreshTime = now;
        }
    }
}

void Lidar::processScanResponse(ScanResponse* response)
{
    qint64 nowMsecs = QDateTime::currentMSecsSinceEpoch();
    qreal angle = LidarTypes::AddDegrees(response->angle(), _offset);

    if(response->quality() > 10 && response->checkBit() == 1 && response->startFlag() == 1 && response->angle() < 360 && response->angle() >= 0)
    {
        qreal offset = angle / _vectorSize;

        if(response->distance() > .010)
        {
            qreal distance = qMax(response->distance(), .001);

            LidarVector* vector = _vectors[int(offset)];
            vector->Vector.Range = distance;
            vector->RefreshTime = nowMsecs;
            _lastGoodSampleTime = nowMsecs;
//KLog::sysLogText(KLOG_DEBUG, "angle: %f  range: %f", angle, distance);
            LidarSample sample(vector->Vector.Bearing, distance, nowMsecs);

            if(vector->Vector.Bearing < _lastBearing && vector->Vector.Bearing < 10)
            {
                KLog::sysLogText(KLOG_INFO, "Scan complete %f < %f", vector->Vector.Bearing, _lastBearing);
            }
            _lastBearing = sample.Vector.Bearing;

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

void Lidar::trimVectors()
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    for(int x = 0;x < _vectors.length();x++)
    {
        if(now > _vectors[x]->RefreshTime + _vectorRefreshTime)
        {
            _vectors[x]->Vector.Range = 0;
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

    KLog::sysLogText(KLOG_DEBUG, "seek to 0x%02x x = 0x%x", b, x);
//    qDebug() << output << " x is " << x << " bytes " << _receiveBuffer.length();
//    KLog::logHex(_receiveBuffer);

    if(x < _bytesInBuffer)
    {
        _bytesProcessed = x + 1;
        result = true;
    }
    else
    {
        qDebug() << "Reset sync";
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

bool Lidar::handleSingleResponse()
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

void Lidar::handleReadyRead()
{
    _readLock.lock();

    QByteArray data = _serialPort->readAll();

    _byteTotal += data.length();

#ifdef DEBUG_SERIAL2
    _dumpOutputFile.write(data);
    _dumpOutputFile.flush();
#endif

    int bytesToAppend = qMin(int(data.length()), int(sizeof(_receiveBuffer) - _bytesInBuffer));
    memcpy(_receiveBuffer + _bytesInBuffer, data.constData(), bytesToAppend);
    _bytesInBuffer += bytesToAppend;

    quint64 now = QDateTime::currentMSecsSinceEpoch();
    if(now > _lastDeliveryMsecs + _bufferMsecs || _bytesInBuffer > _bufferBytes)
    {
        KLog::sysLogText(KLOG_DEBUG, "Delivering %d bytes %lld > %lld + %d || %d > %d",
                         _bytesInBuffer, QDateTime::currentMSecsSinceEpoch(), _lastDeliveryMsecs, _bufferMsecs,
                         _bytesInBuffer, _bufferBytes);
        deliverData();
    }

    _timer->start(1000);

    _readLock.unlock();
}

void Lidar::deliverData()
{
    if(_scanning == true)
    {
        _bytesInBuffer = 0;
    }
    else
    {
        processReadBuffer();
    }
    _lastDeliveryMsecs = QDateTime::currentMSecsSinceEpoch();
    KLog::sysLogText(KLOG_DEBUG, "After delivery %d left",
                     _bytesInBuffer);
}

void Lidar::handleTimeout()
{
    if(_bytesInBuffer > 0)
    {
        KLog::sysLogText(KLOG_DEBUG, "Timeout with %d bytes", _bytesInBuffer);
        deliverData();
    }
    _timer->start(1000);
}

void Lidar::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError)
    {
        qDebug() << "Serial port error " << serialPortError;
    }
}

void Lidar::readyWrite()
{
    qDebug() << "Ready write on thread " << QThread::currentThreadId();
    KLog::sysLogHex(_sendData);
    _serialPort->write(_sendData);
}

void Lidar::handleThreadStarted()
{
    _timer->start(1000);
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

    processReadBuffer();
}

