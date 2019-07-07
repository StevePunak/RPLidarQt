#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>

class ClientThread : public QObject
{
    Q_OBJECT

public:
    explicit ClientThread(int socketDescriptor, QObject *parent = nullptr);
    virtual ~ClientThread();

signals:
    void error(QTcpSocket::SocketError socketError);

public slots:
    void handleRangeMap(QByteArray data);

    void handleReadyRead();
    void handleSocketError(QAbstractSocket::SocketError socketError);
    void handleSocketClosed();

private:
    void init();
    void disconnectSignals();

    int _socketDescriptor;
    QThread _thread;
    QTcpSocket _socket;
};

#endif // CLIENTTHREAD_H
