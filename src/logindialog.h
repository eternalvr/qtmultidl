#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkrequest.h>
#include <QtNetwork/qnetworkreply.h>
#include <QtNetwork/qsslconfiguration.h>
#include <qmessagebox.h>
#include <qjsonobject.h>
#include <qjsondocument.h>
#include <qevent.h>

#include <qpushbutton.h>

#include "netmanager.h"
#include "configuration.h"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();
    bool checkLogin();
    void accept();
    void reject();
    void SetConfiguration(Configuration *config);
    void checkAutoLogin(QString identifier);

    void Initialize();
public slots:
    void onTextChange(QString string);

private slots:
    void onSuccessfulLoginRequest(QNetworkReply *reply);
    void onFailedRequest(QString errorText);

    void onSuccessfulAutologinRequest(QNetworkReply *reply);
private:
    Ui::LoginDialog *ui;
    void disconnectSlots();
    Configuration *config;


    void extractCookie(QNetworkReply *reply);
    void resetForm();
};

#endif // LOGINDIALOG_H
