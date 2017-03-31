#include "deleteworker.h"

DeleteWorker::DeleteWorker()
{
	qDebug() << "[" << QThread::currentThreadId() << "] DeleteWorker::DeletWorker()";
	runMutex = new QMutex();
    deleteQueue = new QQueue<MP3*>();
    workerActive = false;

	intervalTimer = new QTimer();
	
	connect(intervalTimer, &QTimer::timeout, this, &DeleteWorker::run);
	

	intervalTimer->setInterval(10000);
	intervalTimer->start();

	
}

void DeleteWorker::run()
{
	qDebug() << "[" << QThread::currentThreadId() << "] DeleteWorker::run";
	
        
		while(!deleteQueue->isEmpty()){
			runMutex->tryLock(30000);
			currentMP3 = deleteQueue->dequeue();
            if(currentMP3 != NULL){
                
                qDebug() << "Deleting MP3: " << currentMP3->ToString() << endl;
                deleteFromServer(currentMP3);

            }
        }

    
}

void DeleteWorker::AddToQueue(MP3 *mp3)
{
        deleteQueue->enqueue(mp3);
}
void DeleteWorker::connectHooks()
{
	qDebug() << "[+] Hooks";
    NetManager *cInstance = NetManager::GetInstance();
    connect(cInstance, &NetManager::onSuccessfulRequest, this, &DeleteWorker::onSuccessfulDeleteMusicRequest);
    connect(cInstance, &NetManager::onFailedRequest, this, &DeleteWorker::onFailedRequest);
}
void DeleteWorker::disconnectHooks()
{
	qDebug() << "[-] Hooks";
    NetManager *cInstance = NetManager::GetInstance();
    disconnect(cInstance, &NetManager::onSuccessfulRequest, this, &DeleteWorker::onSuccessfulDeleteMusicRequest);
    disconnect(cInstance, &NetManager::onFailedRequest, this, &DeleteWorker::onFailedRequest);
}

bool DeleteWorker::IsActive()
{
    return workerActive;
}

void DeleteWorker::SetActive(bool active)
{
    workerActive = active;
}

void DeleteWorker::onSuccessfulDeleteMusicRequest(QNetworkReply *reply)
{
    disconnectHooks();
    int code = reply->attribute(QNetworkRequest::Attribute::HttpStatusCodeAttribute).toInt();
	qDebug() << "DeleteMusicRequest: Code " << code;
	if(code == 200) {

        QString response(reply->readAll());

        if(response == "true"){
            emit fileDeleted(currentMP3);
            runMutex->unlock();
        }

    }
}
void DeleteWorker::deleteFromServer(MP3 *mp3)
{
    NetManager *cInstance = NetManager::GetInstance();
    connectHooks();
    cInstance->Get(cInstance->GenerateUrl(QString("/system/extdelete/sess/%1").arg(mp3->Session)));
}
void DeleteWorker::onFailedRequest(QString errorText)
{
    disconnectHooks();
    QMessageBox::critical(NULL, "Löschen fehlgeschlagen", "Die Anfrage konnte nicht ausgeführt werden. \n\nFehlermeldung: " + errorText + "\n\nBitte probieren Sie es später erneut." ,QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
    workerActive = false;
    runMutex->unlock();
}

