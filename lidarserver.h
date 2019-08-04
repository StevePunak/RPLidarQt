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
    explicit LidarServer(quint16 listenPort);

    int clientCount() const { return _clientCount; }

protected:
    void incomingConnection(qintptr socketDescriptor) override;

signals:
    void rangeMapReady(QByteArray data);

public slots:
    void handleScanReady(QByteArray rangeData);
    void handleClientStarted();
    void handleClientFinished();

private:
    quint16 _listenPort;
    int _clientCount;
};

#endif // LIDARSERVER_H
