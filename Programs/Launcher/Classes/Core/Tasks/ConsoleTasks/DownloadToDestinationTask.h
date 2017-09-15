#pragma once

#include "Core/Tasks/ConsoleTasks/ConsoleBaseTask.h"

struct ApplicationContext;
struct ConfigHolder;

class DownloadToDestinationTask : public ConsoleBaseTask
{
public:
    DownloadToDestinationTask();
    ~DownloadToDestinationTask();

private:
    QCommandLineOption CreateOption() const override;
    void Run(const QStringList& arguments) override;

    void OnUpdateConfigFinished(const QStringList& arguments);

    ApplicationContext* appContext = nullptr;
    ConfigHolder* configHolder = nullptr;
    QString destPath;
};

Q_DECLARE_METATYPE(DownloadToDestinationTask);
