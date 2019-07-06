#ifndef FILE_INTERFACE_H
#define FILE_INTERFACE_H
#include <QObject>
#include <QString>
#include "deviceinterface.h"

class FileInterface : public DeviceInterface
{
    Q_OBJECT

public:
    FileInterface(const QString& fileName);

    void send(QByteArray& data);

private:
    QString _fileName;
};

#endif // FILE_INTERFACE_H
