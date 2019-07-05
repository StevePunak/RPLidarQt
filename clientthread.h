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

signals:
      void error(QTcpSocket::SocketError socketError);

public slots:
    void scanReady(QByteArray data);

    void handleReadyRead();
    void handleSocketError();
    void handleSocketClosed();

private:
    void init();

    int _socketDescriptor;
    QThread _thread;
    QTcpSocket _socket;
};

#endif // CLIENTTHREAD_H
