#ifndef NETMANAGER_H
#define NETMANAGER_H

#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qsslconfiguration.h>
#include <QtNetwork/qnetworkcookiejar.h>
#include <QtNetwork/qnetworkcookie.h>
#include <qapplication.h>
#include <qjsonobject.h>
#include <qjsondocument.h>
#include <qevent.h>
#include <qstring.h>
#include <qvariant.h>
#include <QThread>
#include "configuration.h"
#include <QNetworkReply>

class NetManager : public QObject
{
Q_OBJECT
public:
    static QThread *NMThread;

    bool debug = false;
    QNetworkAccessManager *manager;
    static NetManager* GetInstance();



	void SetPreferredThread(QThread * thread);

    NetManager();
    ~NetManager();

    void Get(QString url);
    void Post(QString url, QByteArray data);
    void SendRequest(QString url, QString method, QByteArray data);
    void SetConfiguration(Configuration *config);

    QString getAutoLogin();
    QString GenerateUrl(QString path);
signals:
    void onSuccessfulRequest(QNetworkReply *reply);
    void onFailedRequest(QString error_message);

private:
    void debugRequest(QNetworkRequest request, QByteArray data);
    void saveAutoLogin(QString val);
    QSettings *getSettings();
	QThread *preferredThread;
    QNetworkReply *reply;

    Configuration *config;
private slots:
    void syncRequestFinished(QNetworkReply *reply);


    void OnReplyError(QNetworkReply::NetworkError f);
};

#endif // NETMANAGER_H
