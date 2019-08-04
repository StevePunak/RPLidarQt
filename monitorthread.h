#ifndef MONITORTHREAD_H
#define MONITORTHREAD_H
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QString>
#include "lidar.h"
#include "lidarserver.h"

class MonitorThread : public QObject
{
    Q_OBJECT

public:
    MonitorThread(const QString& sourceName, qreal _vectorSize, Lidar::ReaderType readerType, quint16 listenPort, GPIO::Pin motorPin);
    void stop();

private:
    Lidar* _lidar;
    QTimer* _timer;
    QThread _thread;
    LidarServer* _server;

    QString _sourceName;
    qreal _vectorSize;
    Lidar::ReaderType _readerType;
    quint16 _listenPort;
    GPIO::Pin _motorPin;

private slots:
    void handleThreadStarted();
    void handleThreadStopped();
    void handleSerialPortOpened();
    void handleTimerExpired();
};

#endif // MONITORTHREAD_H
