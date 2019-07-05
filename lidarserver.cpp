#include "lidarserver.h"
#include "klog.h"

LidarServer::LidarServer(Lidar* lidar, QObject *parent) :
    QTcpServer(parent),
    _lidar(lidar)
{
    listen(QHostAddress::Any, 5005);
}

void LidarServer::incomingConnection(qintptr socketDescriptor)
{
    ClientThread* thread = new ClientThread(int(socketDescriptor), this);
    connect(this, &LidarServer::scanReady, thread, &ClientThread::scanReady);
}

void LidarServer::handleScanReady(QByteArray data)
{
    KLog::sysLogText(KLOG_DEBUG, "Scan complete <<");
    emit scanReady(data);
}

