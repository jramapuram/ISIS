#ifndef POLLERTHREAD_H
#define POLLERTHREAD_H

#include <QThread>

class PollerThread : public QThread
{
    Q_OBJECT
public:
    PollerThread(QObject *parent = 0);
    ~PollerThread();

signals:
    void pollImage(const QImage& img);

protected:
    void run();

public slots:

};

#endif // POLLERTHREAD_H
