#pragma once

#include <QTCore/QThread>
#include <QDebug>
#include <Windows.h>

class Worker : public QThread
{
    Q_OBJECT

private:

public slots:
    void run() override;

    void killExistingProcess(float end);

signals:
    void progress(float p);
    void description(QString value);
};