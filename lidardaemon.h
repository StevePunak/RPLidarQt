#ifndef LIDARDAEMON_H
#define LIDARDAEMON_H
#include <QObject>
#include <QSocketNotifier>
#include "monitorthread.h"

class LidarDaemon : public QObject
{
    Q_OBJECT

public:
    LidarDaemon(QObject *parent = 0);
    ~LidarDaemon();

    // Unix signal handlers.
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);

    void setMonitorThread(MonitorThread* monitorThread);

public slots:
    // Qt signal handlers.
    void handleSigHup();
    void handleSigTerm();

private:
    static int sighupFd[2];
    static int sigtermFd[2];

    QSocketNotifier *snHup;
    QSocketNotifier *snTerm;

    MonitorThread* _monitorThread;
};

#endif // LIDARDAEMON_H
