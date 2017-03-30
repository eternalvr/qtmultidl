#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H
#include <QRunnable>
#include "mp3.h"
#include <QProgressBar>
#include <QDebug>
#include <QFileInfo>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <stdio.h>
#include <QFile>
#include <QTime>
#include <QDir>
#include <QMutex>

#include "mp3.h"


#include <QNetworkAccessManager>

class DownloadThread : public QObject, public QRunnable
{
    Q_OBJECT

public:
    DownloadThread();
    MP3 *Mp3;
    QProgressBar *progressBar;
    QFile *outputFile;
    QString DownloadDirectory;


    // QThread interface
protected:
    void run();
    QNetworkAccessManager *manager;
    QNetworkRequest *request;
    QNetworkReply *currentDownload;

    QTime downloadTime;
private:
    QString getFileName(MP3 *mp3);
    QMutex *RunMutex;

signals:
    void onStartDownload();
    void onDownloadFinished(MP3 *mp3);

private slots:
    void downloadStart();
    void downloadReadyRead();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
};

#endif // DOWNLOADTHREAD_H
