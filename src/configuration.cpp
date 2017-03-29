#include "configuration.h"

Configuration::Configuration()
{
    settings = new QSettings("ZEEZEE", "MultiDownload");
    Load();
}

void Configuration::Load()
{
    Hostname =  settings->value("hostname","").toString();
    Username =  settings->value("username","").toString();
    AutoLogin = settings->value("autologin","").toString();

    UseSSL = settings->value("use_ssl").toBool();

    SaveDir = settings->value("save_dir","").toString();
    ConcurrentDownloads = settings->value("concurrent_downloads", 4).toInt();
}

void Configuration::Save()
{
    settings->setValue("autologin", AutoLogin);
    settings->setValue("username",  Username);
    settings->setValue("hostname",  Hostname);

    settings->setValue("use_ssl", UseSSL);

    settings->setValue("save_dir", SaveDir);
    settings->setValue("concurrent_downloads", ConcurrentDownloads);
}
