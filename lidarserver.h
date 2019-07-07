#ifndef LIDARSERVER_H
#define LIDARSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QByteArray>
#include <QVector>
#include "clientthread.h"
#include "rangemap.h"

class Lidar;
class LidarServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit LidarServer(Lidar* lidar, quint16 listenPort);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

signals:
    void rangeMapReady(QByteArray data);

public slots:
    void handleScanReady(QByteArray rangeData);

private:
    Lidar* _lidar;
    quint16 _listenPort;
};

#endif // LIDARSERVER_H
