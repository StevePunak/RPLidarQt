#include "deviceinterface.h"

DeviceInterface::DeviceInterface(GPIO::Pin motorPin) :
    QObject(),
    _deviceOpen(false),
    _motorPin(motorPin)
{

}
