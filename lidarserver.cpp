#include "lidarserver.h"
#include "klog.h"

LidarServer::LidarServer(Lidar* lidar, quint16 listenPort) :
    QTcpServer(),
    _lidar(lidar),
    _listenPort(listenPort)
{
    listen(QHostAddress::Any, _listenPort);
}

void LidarServer::incomingConnection(qintptr socketDescriptor)
{
    ClientThread* thread = new ClientThread(int(socketDescriptor), this);
    KLog::sysLogText(KLOG_DEBUG, "Connecting 2");
    connect(this, &LidarServer::rangeMapReady, thread, &ClientThread::handleRangeMap);
    KLog::sysLogText(KLOG_DEBUG, "END Connecting 2");
}

void LidarServer::handleScanReady(QByteArray rangeData)
{
    emit rangeMapReady(rangeData);
}

