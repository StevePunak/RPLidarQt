#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>

class LidarServer;
class ClientThread : public QThread
{
    Q_OBJECT

public:
    explicit ClientThread(int socketDescriptor, LidarServer* parent);
    virtual ~ClientThread();

signals:
    void error(QTcpSocket::SocketError socketError);

public slots:
    void handleRangeMap(QByteArray data);
    void handleReadyRead();
    void handleSocketClosed();

private:
    void init();

    int _socketDescriptor;
    QTcpSocket* _socket;
    LidarServer* _parent;
};

#endif // CLIENTTHREAD_H
