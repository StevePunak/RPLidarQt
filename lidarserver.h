#ifndef LIDARSERVER_H
#define LIDARSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QByteArray>
#include "clientthread.h"

class Lidar;
class LidarServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit LidarServer(Lidar* lidar, QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

signals:
    void scanReady(QByteArray data);

public slots:
    void handleScanReady(QByteArray data);

private:
    Lidar* _lidar;
};

#endif // LIDARSERVER_H
