#pragma once

#include <QTCore/QThread>
#include <QDebug>
#include <Windows.h>
#include <string>
#include <TlHelp32.h>
#include "HTTP.hpp"
#include <fstream>

class Worker : public QThread
{
    Q_OBJECT

private:
    std::string InstallDirectory = "";

public:
    std::string newVersion;

public slots:
    void run() override;

    void killExistingProcess(float end);
    void getNewVersion();

signals:
    void progress(float p);
    void description(QString value);
};