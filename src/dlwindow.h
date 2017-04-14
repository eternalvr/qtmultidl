#ifndef DLWINDOW_H
#define DLWINDOW_H

#include <QMainWindow>
#include "configuration.h"
#include <qapplication.h>
#include <qprocess.h>
#include <QtXml/QDomDocument>
#include <QtXml/QDomElement>
#include <QNetworkReply>
#include <QList>
#include <QProgressBar>
#include <QScrollBar>
#include <QFileDialog>
#include <QThreadPool>
#include <QThread>
#include <QSpinBox>

#include "deleteworker.h"
#include "mp3.h"
#include "downloadthread.h"

namespace Ui {
class DLWindow;
}

class DLWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DLWindow(QWidget *parent = 0);
    ~DLWindow();
    void initialize();
    static bool loggedIn;
    void SetConfiguration(Configuration *config);
    void reloadMyMusic();
signals:
    void cancelDownload(QString session);

private slots:
    void on_pushButton_clicked();

    void onSuccessfulMyMusicRequest(QNetworkReply *reply);

    void onFailedRequest(QString errorText);
    void on_btnBrowse_clicked();

    void on_btnDownloadAll_clicked();

    void on_btnDownloadSelected_clicked();

    void ShowTableContextMenu(const QPoint &point);
    void on_tableWidget_doubleClicked();

    void on_spinDownloads_valueChanged(int arg1);
    void onFileDeleted(MP3 mp3);

    void onSuccessfulDownload(MP3 mp3);
    void onFailedDownload(MP3 mp3);

    void onMenuLogoutClick();

    void onDeleteAllClicked();
    void on_btnDownloadStopDownloads_clicked();

private:
    const int PROGRESSBAR_COLUMN = 3;
    const int SESSION_COLUMN = 4;

    Ui::DLWindow *ui;
    Configuration *config;
    QThreadPool *downloadThreads;
    DeleteWorker *deleteWorker;
    QAction *actionDownloadStart;
    QAction *actionDownloadDelete;
    QAction *actionDownloadCancel;
    QAction *actionDownloadReset;


    void displayXml(QByteArray xml);
    QList<MP3 *> mp3s;
    void populateListView();
    void startDownload(int row);

    void deleteFromServer(MP3 mp3);
    void createContextMenuActions();
    bool findMp3BySession(QString session, MP3* &mp3);
    void MarkRowDisabled(int row);

    void ShowHideCancelButton();
};

#endif // DLWINDOW_H
