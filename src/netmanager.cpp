#include "netmanager.h"

NetManager *NetManager::GetInstance()
{
    static NetManager *instance;
    if(instance == NULL) {
        instance = new NetManager();
    }
    return instance;
}

NetManager::NetManager()
{
    manager = new QNetworkAccessManager();
    manager->setCookieJar(new QNetworkCookieJar());
}
NetManager::~NetManager()
{
    delete(manager);
}
void NetManager::Get(QString url)
{
    SendRequest(url, "GET", NULL);
}

void NetManager::Post(QString url, QByteArray data)
{
    SendRequest(url, "POST", data);
}

void NetManager::SendRequest(QString url, QString method, QByteArray data)
{

    qDebug() << "Sending request to: " << url << endl;
    QNetworkRequest request;
    QNetworkReply *reply = NULL;

    QSslConfiguration config = QSslConfiguration::defaultConfiguration();

    if(url.startsWith("https")) {
        config.setProtocol(QSsl::TlsV1_2);
        request.setSslConfiguration(config);
    }

    request.setUrl(QUrl(url));
    request.setRawHeader("X-Requested-With", "XMLHttpRequest");
    request.attribute(request.Attribute::FollowRedirectsAttribute, false);
    connect(manager, &QNetworkAccessManager::finished, this, &NetManager::syncRequestFinished);

    if(method == "POST") {
        request.setHeader(QNetworkRequest::KnownHeaders::ContentTypeHeader, "application/x-www-form-urlencoded");
        reply = manager->post(request, data);
    }
    if(method =="GET") {
        reply = manager->get(request);
    }

    if(debug == true){
        debugRequest(request, data);
    }
}

void NetManager::SetConfiguration(Configuration *config)
{
    this->config = config;
}

void NetManager::debugRequest(QNetworkRequest request, QByteArray data)
{
      qDebug() << "REQUEST:" << endl;
      const QList<QByteArray>& rawHeaderList(request.rawHeaderList());
      foreach (QByteArray rawHeader, rawHeaderList) {
        qDebug() << rawHeader << ": " << request.rawHeader(rawHeader);
      }
      qDebug() << data;

}

QString NetManager::GenerateUrl(QString path)
{
    QString schema = (config->UseSSL == true) ? "https://" : "http://";
    return QString(schema + config->Hostname + "/" + path);
}

void NetManager::syncRequestFinished(QNetworkReply *reply)
{
     disconnect(manager, &QNetworkAccessManager::finished, this, &NetManager::syncRequestFinished);

    if(reply->error() == QNetworkReply::NetworkError::NoError) {

//        QByteArray response = reply->readAll();


        qDebug() << "Return code: " << reply->attribute(QNetworkRequest::Attribute::HttpStatusCodeAttribute).toInt() << endl;

        emit onSuccessfulRequest(reply);

    } else {
        qDebug() << "Network error: " << reply->error() << ": "  << reply->errorString();
        QString errorMsg = reply->errorString();
        emit onFailedRequest(errorMsg);
    }

}

