#ifndef DEVICE_INTERFACE_H
#define DEVICE_INTERFACE_H
#include <QObject>
#include <QByteArray>
#include "gpio.h"

class DeviceInterface : public QObject
{
    Q_OBJECT
public:
    explicit DeviceInterface(GPIO::Pin motorPin);

    virtual void send(QByteArray& data) = 0;
    virtual void startMotor() {}
    virtual void stopMotor() {}

    bool deviceOpen() const { return _deviceOpen; }
    GPIO::Pin motorPin() const { return _motorPin; }

protected:
    bool _deviceOpen;
    GPIO::Pin _motorPin;

signals:
    void deviceOpened();
    void dataReady(QByteArray data);
};

#endif // DEVICE_INTERFACE_H
