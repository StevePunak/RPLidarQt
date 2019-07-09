#ifndef SERIAL_INTERFACE_H
#define SERIAL_INTERFACE_H
#include <QSerialPort>
#include <QObject>
#include "deviceinterface.h"

class SerialInterface : public DeviceInterface
{
    Q_OBJECT
public:
    explicit SerialInterface(const QString& portName, bool initPort, GPIO::Pin motorPin);

    virtual void startMotor();
    virtual void stopMotor();

protected:
    QString _portName;
    QSerialPort* _serialPort;
    int _byteTotal;

private:
    void initializeSerialPort();

signals:

public slots:
};

#endif // SERIAL_INTERFACE_H
