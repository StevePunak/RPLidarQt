#ifndef DATAREADER_H
#define DATAREADER_H

#include <QObject>
#include <QByteArray>


class DataReader : public QObject
{
    Q_OBJECT
public:
    explicit DataReader();

    virtual void send(QByteArray& data) = 0;

signals:
    void dataReady(QByteArray data);

public slots:
};

#endif // DATAREADER_H
