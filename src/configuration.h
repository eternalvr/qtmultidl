#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QObject>
#include <qsettings.h>

class Configuration
{
public:
    Configuration();

    QString Hostname;
    QString Username;
    QString AutoLogin;
    QString SaveDir;
    int ConcurrentDownloads;


    bool UseSSL;

    QSettings *settings;

    void Load();
    void Save();
};

#endif // CONFIGURATION_H
