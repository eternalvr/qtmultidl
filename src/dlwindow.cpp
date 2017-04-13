#include "dlwindow.h"
#include "ui_dlwindow.h"
#include "logindialog.h"

DLWindow::DLWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DLWindow)
{
    ui->setupUi(this);
    ui->pbPrefab->setVisible(false);

    createContextMenuActions();
}

void DLWindow::createContextMenuActions()
{
    actionDownloadCancel = new QAction("Download abbrechen");
    actionDownloadDelete = new QAction("Aus dem Speicherplatz löschen");
    actionDownloadStart = new QAction("Download starten");
    actionDownloadReset = new QAction("Download zurücksetzen");

    connect(this->ui->action_Ausloggen, &QAction::triggered, this, &DLWindow::onMenuLogoutClick);
    connect(this->ui->action_Beenden, &QAction::triggered, this, &DLWindow::close);
    connect(this->ui->actionAlles_Herunterladen, &QAction::triggered, this, &DLWindow::on_btnDownloadAll_clicked);
    connect(this->ui->actionAlles_L_schen, &QAction::triggered, this, &DLWindow::onDeleteAllClicked);
}
void DLWindow::onDeleteAllClicked()
{
    QMessageBox *msgBox = new QMessageBox(QMessageBox::Icon::Question, "ACHTUNG!", "Es werden _ALLE_ in der Liste angezeigten Titel aus dem Onlinespeicher gelöscht. Möchten Sie das wirklich?", QMessageBox::Yes|QMessageBox::No);
    msgBox->setButtonText(QMessageBox::Button::Yes, "Ja");
    msgBox->setButtonText(QMessageBox::Button::No, "Nein");
    if(QMessageBox::Yes == msgBox->exec()){
        // we don't do this
    }
}
void DLWindow::onMenuLogoutClick()
{
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    config->AutoLogin = "";
    config->Save();

    this->close();
}

void DLWindow::initialize()
{
	qDebug() << "[" << QThread::currentThread() << "] DLWindow::Initialize";
    downloadThreads = new QThreadPool(this);
    downloadThreads->setMaxThreadCount(config->ConcurrentDownloads);

    deleteWorker = new DeleteWorker();
	deleteWorker->setParent(this);

	NetManager::GetInstance()->setParent(this);
		

	connect(deleteWorker, &DeleteWorker::fileDeleted, this, &DLWindow::onFileDeleted);

    ui->txtSaveDir->setText(config->SaveDir);
    ui->spinDownloads->setValue(config->ConcurrentDownloads);

    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &DLWindow::ShowTableContextMenu);

    reloadMyMusic();
}
void DLWindow::MarkRowDisabled(int row)
{
    for(int i = 0; i < ui->tableWidget->columnCount();i++)
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, i);
        if(item != NULL) {
            item->setFlags(!Qt::ItemIsSelectable & !Qt::ItemIsEnabled );
        }
    }
}

void DLWindow::ShowTableContextMenu(const QPoint &point)
{
        QPoint globalPos = ui->tableWidget->viewport()->mapToGlobal(point);

        QMenu myMenu;

        QList<QTableWidgetItem *> items = ui->tableWidget->selectedItems();

        if(items.count() > 0) {

            QProgressBar *pb = (QProgressBar*) ui->tableWidget->cellWidget(items.at(0)->row(), PROGRESSBAR_COLUMN);

            if(pb->value() <= 0) {
                myMenu.addAction(actionDownloadStart);
            } else if (pb->value() > 0 && pb->value() < pb->maximum()) {
                myMenu.addAction(actionDownloadCancel);
                myMenu.addAction(actionDownloadReset);
            } else {
                myMenu.addAction(actionDownloadReset);
            }


            myMenu.addSeparator();
            myMenu.addAction(actionDownloadDelete);
        }

        QAction* selectedItem = myMenu.exec(globalPos);
        if (selectedItem)
        {
            if(selectedItem == actionDownloadStart){
                on_btnDownloadSelected_clicked();
            }
            if(selectedItem == actionDownloadDelete){
                QList<QTableWidgetItem *> selectedItems = ui->tableWidget->selectedItems();
                foreach (QTableWidgetItem *item, selectedItems) {
                    QString session = ui->tableWidget->item(item->row(), 4)->text();
                    if(item->column() == 0) {
                        MP3 *mp3 = NULL;
                        if(findMp3BySession(session, mp3)){
                            QMessageBox *msgBox = new QMessageBox(QMessageBox::Icon::Question, "Vom Server löschen", "Soll der Titel \n\n" + mp3->Artist + " - " + mp3->Track + "\n\nwirklich vom Server gelöscht werden?", QMessageBox::Yes|QMessageBox::No);
                            msgBox->setButtonText(QMessageBox::Button::Yes, "Ja");
                            msgBox->setButtonText(QMessageBox::Button::No, "Nein");
                            if(QMessageBox::Yes == msgBox->exec()){
                                QProgressBar *pb = static_cast<QProgressBar *>(ui->tableWidget->cellWidget(item->row(),PROGRESSBAR_COLUMN));
                                pb->setFormat("wird gelöscht..");
                                pb->setRange(0,0);
                                pb->setValue(-1);
                                pb->setEnabled(true);
                                deleteFromServer(*mp3);
                                MarkRowDisabled(item->row());
                            }
                            msgBox->deleteLater();
                        }
                    }
                }

            }
            if(selectedItem == actionDownloadReset) {
                QList<QTableWidgetItem *> selectedItems = ui->tableWidget->selectedItems();
                foreach (QTableWidgetItem *item, selectedItems) {
                    ((QProgressBar *)ui->tableWidget->cellWidget(item->row(), PROGRESSBAR_COLUMN))->setValue(0);
                    cancelDownload(ui->tableWidget->item(item->row(), SESSION_COLUMN)->text());
                }
            }
            if(selectedItem == actionDownloadCancel) {
                QList<QTableWidgetItem *> selectedItems = ui->tableWidget->selectedItems();
                foreach (QTableWidgetItem *item, selectedItems) {
                    if(item->column() == 0) {
                        emit cancelDownload(ui->tableWidget->item(item->row(), 4)->text());
                    }
                }
            }
        }

}

void DLWindow::SetConfiguration(Configuration *config)
{
    this->config = config;
}

DLWindow::~DLWindow()
{
    config->Save();
    delete ui;
}

void DLWindow::on_pushButton_clicked()
{
    reloadMyMusic();
}

void DLWindow::reloadMyMusic()
{
   NetManager *cInstance = NetManager::GetInstance(); 

   connect(cInstance, &NetManager::onSuccessfulRequest, this, &DLWindow::onSuccessfulMyMusicRequest);
   connect(cInstance, &NetManager::onFailedRequest, this, &DLWindow::onFailedRequest);

   cInstance->Get(cInstance->GenerateUrl("mymusic?format=xml"));
}

void DLWindow::displayXml(QByteArray xml)
{
    QDomDocument domdoc;
    domdoc.setContent(xml);

    QDomNodeList nodeList = domdoc.elementsByTagName("record");
    int elements = nodeList.count();

    if(elements == 0) {
        //QMessageBox::warning(this, "Fehler beim Abruf", "Die Anfrage konnte nicht ausgeführt werden. \n\n Bitte probieren Sie es später erneut." ,QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
        ui->lblLoading->setText("Es konnten keine Musikstücke gefunden werden. Bitte probieren Sie es später erneut.");
        return;
    }
    mp3s.clear();


    for(int i = 0; i < elements; i++){
        MP3 *mp3 = new MP3(nodeList.at(i).firstChildElement("artist").text(),
                nodeList.at(i).firstChildElement("track").text(),
                nodeList.at(i).firstChildElement("url").text(),
                nodeList.at(i).firstChildElement("session").text());

        mp3s.append(mp3);
    }

    populateListView();
}
bool DLWindow::findMp3BySession(QString session, MP3* &mp3)
{
    if(mp3s.count() == 0) {
        return false;
    }

    for(int i = 0; i < mp3s.count(); i++) {
        if(mp3s.at(i)->Session == session){
          mp3 = (mp3s.at(i));
          return true;
        }
    }
    return false;
}

void DLWindow::populateListView()
{
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setRowCount(mp3s.count());

    ui->tableWidget->setColumnHidden(2, true); // hide url
    ui->tableWidget->setColumnHidden(SESSION_COLUMN, true); // hide session


    QStringList labels("Interpret");
    labels.append("Titel");
    labels.append("");

    ui->tableWidget->setHorizontalHeaderLabels(labels);

    int cellwidth = ui->tableWidget->width()/3 - 8; // scientific constant!

    ui->tableWidget->setColumnWidth(0, cellwidth);
    ui->tableWidget->setColumnWidth(1, cellwidth);
    ui->tableWidget->setColumnWidth(3, cellwidth);



    int row = 0;
    for( MP3 *mp3 : mp3s){
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(mp3->Artist));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(mp3->Track));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(mp3->Url));
        ui->tableWidget->setItem(row, 4, new QTableWidgetItem(mp3->Session));

        QProgressBar *qp = new QProgressBar(ui->tableWidget);
        qp->setStyleSheet(ui->pbPrefab->styleSheet());
        qp->setFont(ui->pbPrefab->font());
        qp->setVisible(true);
        qp->setTextVisible(true);

        ui->tableWidget->setCellWidget(row, PROGRESSBAR_COLUMN, qp);
        //qp->resize(cellwidth, 15);




        row++;
    }
    this->ui->lblLoading->setVisible(false);
}

void DLWindow::onSuccessfulMyMusicRequest(QNetworkReply *reply)
{
    NetManager *cInstance = NetManager::GetInstance();
    disconnect(cInstance, &NetManager::onSuccessfulRequest, this, &DLWindow::onSuccessfulMyMusicRequest);
    disconnect(cInstance, &NetManager::onFailedRequest, this, &DLWindow::onFailedRequest);

    int code = reply->attribute(QNetworkRequest::Attribute::HttpStatusCodeAttribute).toInt();
    if(code == 200) {

        displayXml(reply->readAll());

    } else if(code == 302) {
        QMessageBox::critical(this, "Fehler beim Abruf", "Sie wurden vom System ausgeloggt. Bitte loggen sie sich erneut ein." ,QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);

        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
        this->close();

    }else {
        QMessageBox::critical(this, "Fehler beim Abruf", "Die Anfrage konnte nicht ausgeführt werden. \n\n Bitte probieren Sie es später erneut." ,QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
    }

}


void DLWindow::onFailedRequest(QString errorText)
{
    NetManager *cInstance = NetManager::GetInstance();
    disconnect(cInstance, &NetManager::onSuccessfulRequest, this, &DLWindow::onSuccessfulMyMusicRequest);
    disconnect(cInstance, &NetManager::onFailedRequest, this, &DLWindow::onFailedRequest);

    QMessageBox::critical(this, "Fehler beim Login", "Die Anfrage konnte nicht ausgeführt werden. \n\nFehlermeldung: " + errorText + "\n\nBitte probieren Sie es später erneut." ,QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
}

void DLWindow::on_btnBrowse_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if(dir != ""){
        ui->txtSaveDir->setText(dir);
        config->SaveDir = dir;
        config->Save();
    }

}
void DLWindow::startDownload(int row)
{
    DownloadThread *dlT = new DownloadThread();
    connect(dlT, &DownloadThread::onDownloadFinished, this, &DLWindow::onSuccessfulDownload);
    connect(dlT, &DownloadThread::onDownloadCancelled, this, &DLWindow::onFailedDownload);


    QString session = ui->tableWidget->item(row, 4)->text();

    qDebug() << "Starting download for row " << row << endl;

    for(int i = 0; i < mp3s.count(); i++) {
        if(mp3s.at(i)->Session == session){
           dlT->Mp3 = *mp3s.at(i);
           dlT->progressBar = (QProgressBar *)ui->tableWidget->cellWidget(row, PROGRESSBAR_COLUMN);
           dlT->DownloadDirectory = config->SaveDir;           
           if(dlT->progressBar->value() == dlT->progressBar->maximum()){
               continue;
           }
           connect(this, &DLWindow::cancelDownload, dlT, &DownloadThread::onCancelDownload);

           downloadThreads->start(dlT);
        }
    }
}

void DLWindow::on_btnDownloadAll_clicked()
{
    if(mp3s.count() == 0 ) {
        QMessageBox::critical(this, "Download nicht möglich", "Sie haben keine MP3s in Ihrem Speicher, die Sie momentan herunterladen können. Probieren Sie es später erneut." ,QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
        return;
    }

    if(ui->txtSaveDir->text() == ""){
        on_btnBrowse_clicked();
        return;
    }


    on_btnDownloadSelected_clicked();
    for(int i = 0; i < ui->tableWidget->rowCount();i++)
     startDownload(i);



}


void DLWindow::on_btnDownloadSelected_clicked()
{
    if(mp3s.count() == 0 ) {
        QMessageBox::critical(this, "Download nicht möglich", "Sie haben keine MP3s in Ihrem Speicher, die Sie momentan herunterladen können. Probieren Sie es später erneut." ,QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
        return;
    }

    if(ui->txtSaveDir->text() == ""){
        on_btnBrowse_clicked();
        return;
    }


    QList<QTableWidgetItem *> selectedItems = ui->tableWidget->selectedItems();
    foreach (QTableWidgetItem *item, selectedItems) {
        if(item->column() == 0) {
            startDownload(item->row());
        }
    }


}

void DLWindow::on_tableWidget_doubleClicked()
{
   on_btnDownloadSelected_clicked();
}

void DLWindow::on_spinDownloads_valueChanged(int arg1)
{
    if(arg1 > 0){
        downloadThreads->setMaxThreadCount(arg1);
        config->ConcurrentDownloads = arg1;
    }
}

void DLWindow::onFileDeleted(MP3 mp3)
{
    for(int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        if(ui->tableWidget->item(i, 4)->text() == mp3.Session) {
            ui->tableWidget->removeRow(i);
            return;
        }
    }
}

void DLWindow::onSuccessfulDownload(MP3 mp3)
{
    qDebug() << "Download successful: " << mp3.ToString() << endl;
    if(ui->chkDeleteSuccessful->isChecked()) {
        deleteFromServer(mp3);
    }
}

void DLWindow::onFailedDownload(MP3 mp3)
{
    qDebug() << "Download NOT successful: " << mp3.ToString() << endl;
}
void DLWindow::deleteFromServer(MP3 mp3)
{
    qDebug() << "Deleting from Server: " << mp3.ToString() << endl;

    deleteWorker->AddToQueue(mp3);
}
