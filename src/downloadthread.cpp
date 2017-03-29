#include "downloadthread.h"

DownloadThread::DownloadThread()
{
    manager = new QNetworkAccessManager();
    RunMutex = new QMutex(QMutex::NonRecursive);
}

void DownloadThread::run()
{

       QString filename = getFileName(this->Mp3);

       outputFile = new QFile(filename);
       if (!outputFile->open(QIODevice::WriteOnly)) {
           qDebug() << "Error opening file: " << filename << endl;

           return;
       }
       request = new QNetworkRequest(Mp3->Url);

       connect(this, &DownloadThread::onStartDownload, this, &DownloadThread::downloadStart);
       RunMutex->lock();

       emit onStartDownload();

       RunMutex->tryLock(20000000);
}

QString DownloadThread::getFileName(MP3 *mp3)
{

    QString basename = this->DownloadDirectory + QDir::separator() + mp3->Artist + " - " + mp3->Track + ".mp3";


    if (QFile::exists(basename)) {
        int i = 0;
        basename += '.';
        while (QFile::exists(basename + QString::number(i)))
            ++i;

        basename += QString::number(i);
    }

    return basename;
}

void DownloadThread::downloadStart()
{
    currentDownload = manager->get(*request);
    connect(currentDownload, &QNetworkReply::downloadProgress, this, &DownloadThread::downloadProgress);
    connect(currentDownload, &QNetworkReply::finished, this, &DownloadThread::downloadFinished);
    connect(currentDownload, &QNetworkReply::readyRead, this, &DownloadThread::downloadReadyRead);
    downloadTime.start();
}

void DownloadThread::downloadReadyRead()
{
    outputFile->write(currentDownload->readAll());
}

void DownloadThread::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    progressBar->setRange(0, bytesTotal);
    progressBar->setValue(bytesReceived);

    double speed = bytesReceived * 1000.0 / downloadTime.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/s";
    } else if (speed < 1024*1024) {
        speed /= 1024;
        unit = "kB/s";
    } else {
        speed /= 1024*1024;
        unit = "MB/s";
    }

    QString format = QString::fromLatin1("%p% %1 %2")
            .arg(speed, 3, 'f', 1).arg(unit);

    progressBar->setFormat(format);
    progressBar->update();
}
void DownloadThread::downloadFinished()
{
    progressBar->setValue(progressBar->maximum());
    progressBar->setFormat("%p%");
    outputFile->close();

    outputFile->deleteLater();

    if (currentDownload->error()) {
        progressBar->setFormat("Download Error");
    }

    currentDownload->deleteLater();
    RunMutex->unlock();
}
