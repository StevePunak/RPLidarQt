#ifndef DEVICE_INTERFACE_H
#define DEVICE_INTERFACE_H

#include <QObject>
#include <QByteArray>


class DeviceInterface : public QObject
{
    Q_OBJECT
public:
    explicit DeviceInterface();

    virtual void send(QByteArray& data) = 0;
    virtual void startMotor() {}
    virtual void stopMotor() {}

    void setDeliverData(bool value) { _deliverData = value; }
    bool deliverData() const { return _deliverData; }

protected:
    bool _deliverData;

signals:
    void dataReady(QByteArray data);

public slots:
};

#endif // DEVICE_INTERFACE_H
