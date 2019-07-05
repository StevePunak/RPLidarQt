#ifndef DATAREADER_H
#define DATAREADER_H

#include <QObject>

class DataReader : public QObject
{
    Q_OBJECT
public:
    explicit DataReader(QObject *parent = nullptr);

signals:

public slots:
};

#endif // DATAREADER_H
