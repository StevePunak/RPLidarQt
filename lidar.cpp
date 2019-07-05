#undef DEBUG_SERIAL2
#include <QDebug>
#include "lidar.h"
#include "lidarprotocol.h"
#include "blockingserialreader.h"
#include "asynchserialreader.h"
#include "filedatareader.h"
#include "klog.h"

Lidar::Lidar(const QString& sourceName, qreal _vectorSize, ReaderType type) :
    _vectorSize(_vectorSize),
    _sourceName(sourceName),
    _offset(0),
    _bytesProcessed(0),
    _bytesInBuffer(0),
    _lastScanOffset(0),
    _lastGoodSampleTime(0),
    _lastBearing(0),
    _scanning(false),
    _server(nullptr)
{
    for(qreal bearing = 0;bearing < 360;bearing += _vectorSize)
    {
        LidarVector* vector = new LidarVector(bearing, 0);
        _vectors.append(vector);
    }

    _vectorRefreshTime = 500;
    _state = Sync;

    switch(type)
    {
    case BlockingSerial:
        _reader = new BlockingSerialReader(sourceName);
        break;

    case AsynchSerial:
        _reader = new AsynchSerialReader(sourceName);
        break;

    case BinaryFile:
        _reader = new FileDataReader(sourceName);
        break;

    }

    //loadTestData();

    init();
}

void Lidar::init()
{
    _server = new LidarServer(this, this);

    connect(this, &Lidar::scanComplete, _server, &LidarServer::handleScanReady);
    connect(this, &Lidar::handleDataReady, _reader, &DataReader::dataReady);

    _reader->moveToThread(&_thread);
    _thread.start();
}

bool Lidar::GetDeviceInfo()
{
    qDebug() << "Clear Start";

    QWaitCondition wait;
    QMutex mutex;
    mutex.lock();
    wait.wait(&mutex, 500);
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
    _reader->send(command.Serialize());
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
        if(_responseQueue.count() > 0)
        {
            response = _responseQueue.takeFirst();
        }
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

void Lidar::handleDataReady(QByteArray data)
{
    int bytesToAppend = qMin(int(data.length()), int(sizeof(_receiveBuffer) - _bytesInBuffer));
    memcpy(_receiveBuffer + _bytesInBuffer, data.constData(), bytesToAppend);
    _bytesInBuffer += bytesToAppend;
    processReadBuffer();
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

