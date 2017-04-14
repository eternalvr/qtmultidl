#include "downloadthread.h"

DownloadThread::DownloadThread()
{
    manager = new QNetworkAccessManager();
    RunMutex = new QMutex(QMutex::NonRecursive);
}

DownloadThread::~DownloadThread()
{
    disconnect();
}

void DownloadThread::run()
{



       outputFile = new QTemporaryFile();
       if (!outputFile->open()) {
           qDebug() << "Error opening file: " << outputFile->fileName() << endl;

           return;
       }
       request = new QNetworkRequest(Mp3.Url);

       connect(this, &DownloadThread::onStartDownload, this, &DownloadThread::downloadStart);
       RunMutex->lock();

       emit onStartDownload();

       RunMutex->tryLock(20000000);
}



QString DownloadThread::getFileName(MP3 *mp3)
{

    QString basename = this->DownloadDirectory + QDir::separator() + mp3->Artist + " - " + mp3->Track;



    if (QFile::exists(basename + ".mp3")) {
        int i = 0;
        basename += '.';
        while (QFile::exists(basename + QString::number(i) + ".mp3"))
            ++i;

        basename += QString::number(i);
    }

    return basename + ".mp3";
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
    qDebug() << "[" << QThread::currentThread() << "] DownloadThread::downloadFinished";
    progressBar->setValue(progressBar->maximum());
    progressBar->setFormat("%p%");


    currentDownload->deleteLater();


    if (currentDownload->error()) {
        outputFile->remove();
        emit onDownloadCancelled(this->Mp3);
        progressBar->setValue(0);
        progressBar->setFormat("Abgebrochen");
        RunMutex->unlock();
        return;
    }
    outputFile->close();
    outputFile->rename(getFileName(&this->Mp3));
    outputFile->setAutoRemove(false);
    outputFile->deleteLater();


    emit onDownloadFinished(this->Mp3);
    RunMutex->unlock();
}

void DownloadThread::onCancelDownload(QString session)
{
    qDebug() << "[" << QThread::currentThread() << "] DownloadThread::onCancelDownload";
    if(this->Mp3.Session == session || session == "-1"){
    if(currentDownload != NULL) {
        if(currentDownload->isRunning())
            this->currentDownload->abort();
        }
    }
}
