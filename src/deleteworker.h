#ifndef DELETEWORKER_H
#define DELETEWORKER_H

#include <QObject>
#include <QRunnable>
#include <QMutex>
#include <QThread>
#include <QQueue>
#include <QNetworkReply>
#include <QMessageBox>

#include <qtimer.h>


#include "netmanager.h"
#include "mp3.h"
class DeleteWorker : public QThread
{
	Q_OBJECT
public:
	DeleteWorker();
	void run();
    void AddToQueue(MP3 mp3);

private:
	QMutex *runMutex;
    QQueue<MP3> *deleteQueue;
    MP3 currentMP3;
	bool workerActive;

	void connectHooks();
	void disconnectHooks();

	bool IsActive();
	void SetActive(bool active);

    void deleteFromServer(MP3 mp3);
	QTimer *intervalTimer;

private slots:
	void onFailedRequest(QString errorText);
	void onSuccessfulDeleteMusicRequest(QNetworkReply *reply);

signals:
    void fileDeleted(MP3 mp3);
};

#endif // DELETEWORKER_H
