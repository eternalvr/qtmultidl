#include "logindialog.h"
#include "ui_logindialog.h"


LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

}
void LoginDialog::Initialize()
{
    resetForm();

    QString autologinIdentifier = config->AutoLogin;

    if(autologinIdentifier == ""){
        ui->progressBar->setVisible(false);
    } else {
        checkAutoLogin(autologinIdentifier);
    }

}

LoginDialog::~LoginDialog()
{
    delete ui;
}
void LoginDialog::accept()
{
    config->Hostname = ui->txtHost->text().trimmed();
    config->UseSSL = (ui->chkSSL->checkState() == Qt::CheckState::Checked);
    checkLogin();
}
void LoginDialog::reject() {

    QDialog::reject();
}
void LoginDialog::resetForm()
{

    disconnect(ui->txtUser, &QLineEdit::textChanged, this, &LoginDialog::onTextChange);
    disconnect(ui->txtPassword, &QLineEdit::textChanged, this, &LoginDialog::onTextChange);
    disconnect(ui->txtHost, &QLineEdit::textChanged, this, &LoginDialog::onTextChange);

    connect(ui->txtUser, &QLineEdit::textChanged, this, &LoginDialog::onTextChange);
    connect(ui->txtPassword, &QLineEdit::textChanged, this, &LoginDialog::onTextChange);
    connect(ui->txtHost, &QLineEdit::textChanged, this, &LoginDialog::onTextChange);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    ui->txtHost->setEnabled(true);
    ui->txtUser->setEnabled(true);
    ui->txtPassword->setEnabled(true);
    ui->chkSSL->setEnabled(true);

    ui->txtUser->setFocus();

    if(config->Username != ""){
        ui->txtUser->setText(config->Username);
        ui->txtPassword->setFocus();
    }

    if(config->Hostname != "") {
        ui->txtHost->setText(config->Hostname);
    }

    ui->chkSSL->setChecked(config->UseSSL);

}

void LoginDialog::SetConfiguration(Configuration *config)
{
    this->config = config;
}


bool LoginDialog::checkLogin()
{
    qDebug() << "checkLogin" << endl;
    ui->progressBar->setVisible(true);
    QString q = "username=%1&password=%2&autologin=1";
            q=q.arg(ui->txtUser->text()).arg(ui->txtPassword->text());

    QByteArray data(q.toLocal8Bit());

    NetManager *cInstance = NetManager::GetInstance();

    connect(cInstance, &NetManager::onSuccessfulRequest, this, &LoginDialog::onSuccessfulLoginRequest);
    connect(cInstance, &NetManager::onFailedRequest, this, &LoginDialog::onFailedRequest);

    cInstance->Post(cInstance->GenerateUrl("login"), data);

    return true;

}
void LoginDialog::checkAutoLogin(QString identifier)
{
    NetManager *cInstance = NetManager::GetInstance();

    ui->progressBar->setVisible(true);

    ui->txtUser->setEnabled(false);
    ui->txtPassword->setEnabled(false);
    ui->txtHost->setEnabled(false);
    ui->chkSSL->setEnabled(false);


    QUrl url(cInstance->GenerateUrl("profile"));
    QNetworkCookie cookie(QString("_zal").toLocal8Bit(), identifier.toLocal8Bit());
    QList<QNetworkCookie> cookieList;

    cookieList.append(cookie);



    cInstance->manager->cookieJar()->setCookiesFromUrl(cookieList, url);

    connect(cInstance, &NetManager::onSuccessfulRequest, this, &LoginDialog::onSuccessfulAutologinRequest);
    connect(cInstance, &NetManager::onFailedRequest, this, &LoginDialog::onFailedRequest);

    cInstance->Get(url.toString());



}
void LoginDialog::onSuccessfulAutologinRequest(QNetworkReply *reply)
{
    disconnectSlots();
    int code = reply->attribute(QNetworkRequest::Attribute::HttpStatusCodeAttribute).toInt();
    if(code == 200) {
        QDialog::accept();
    } else {
        QMessageBox::critical(this, "Login", "Ihr Automatisches Login ist abgelaufen. Bitte loggen Sie sich neu ein.", QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
        resetForm();

    }
}

void LoginDialog::disconnectSlots()
{
    NetManager *cInstance = NetManager::GetInstance();
    disconnect(cInstance, &NetManager::onSuccessfulRequest, this, &LoginDialog::onSuccessfulLoginRequest);
    disconnect(cInstance, &NetManager::onFailedRequest, this, &LoginDialog::onFailedRequest);
    disconnect(cInstance, &NetManager::onSuccessfulRequest, this, &LoginDialog::onSuccessfulAutologinRequest);
    disconnect(cInstance, &NetManager::onFailedRequest, this, &LoginDialog::onFailedRequest);

    ui->progressBar->setVisible(false);
}
void LoginDialog::onSuccessfulLoginRequest(QNetworkReply *reply)
{
    disconnectSlots();
    qDebug() << "Request successful" << endl;
    QString body(reply->readAll());

    qDebug() << body << endl;
    QJsonDocument json = QJsonDocument::fromJson(body.toLocal8Bit());
    QJsonObject jsonO = json.object();

    if(jsonO["status"].toString().startsWith("success")){
        config->Hostname = ui->txtHost->text().trimmed();
        config->Username = ui->txtUser->text().trimmed();

        extractCookie(reply);

        config->Save();
        QDialog::accept();

    } else {
        QMessageBox::critical(this, "Fehler beim Login", jsonO["message"].toString() ,QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
    }
}
void LoginDialog::onFailedRequest(QString errorText)
{
    disconnectSlots();
    QMessageBox::critical(this, "Fehler beim Login", "Die Anfrage konnte nicht ausgeführt werden. \n\nFehlermeldung: " + errorText + "\n\nBitte loggen Sie sich erneut ein." ,QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
    resetForm();

}
void LoginDialog::onTextChange(QString string)
{
    if(ui->txtUser->text().trimmed() == "" || ui->txtPassword->text().trimmed() == "" || ui->txtHost->text().trimmed() == "") {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    } else {
        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
    }
}
void LoginDialog::extractCookie(QNetworkReply *reply)
{
    QVariant cookieVar = reply->header(QNetworkRequest::SetCookieHeader);
    if (cookieVar.isValid()) {
        QList<QNetworkCookie> cookies = cookieVar.value<QList<QNetworkCookie> >();
        foreach (QNetworkCookie cookie, cookies) {
            //qDebug() << cookie.name() << " = " << cookie.value() << endl;
            if(cookie.name()=="_zal") {
                config->AutoLogin = cookie.value();
            }
        }
    }
}
