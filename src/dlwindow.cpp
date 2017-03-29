#include "dlwindow.h"
#include "ui_dlwindow.h"
#include "logindialog.h"

DLWindow::DLWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DLWindow)
{
    ui->setupUi(this);
    ui->pbPrefab->setVisible(false);

}
void DLWindow::initialize()
{
    downloadThreads = new QThreadPool(this);
    downloadThreads->setMaxThreadCount(5);

    ui->txtSaveDir->setText(config->SaveDir);

    connect(ui->tableWidget, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(ShowTableContextMenu(const QPoint&)));

    reloadMyMusic();
}
void DLWindow::ShowTableContextMenu(const QPoint &point)
{
        QPoint globalPos = ui->tableWidget->viewport()->mapToGlobal(point);

        QMenu myMenu;

        QList<QTableWidgetItem *> items = ui->tableWidget->selectedItems();

        if(items.count() > 0) {

            QProgressBar *pb = (QProgressBar*) ui->tableWidget->cellWidget(items.at(0)->row(), 3);

            if(pb->value() <= 0) {
                myMenu.addAction("Download starten");
            } else if (pb->value() > 0 && pb->value() < pb->maximum()) {
                myMenu.addAction("Download abbrechen");
                myMenu.addAction("Download zurücksetzen");
            } else {
                myMenu.addAction("Download zurücksetzen");
            }


            myMenu.addSeparator();
            myMenu.addAction("Aus dem Speicherplatz löschen");
        }






        QAction* selectedItem = myMenu.exec(globalPos);
        if (selectedItem)
        {
            // something was chosen, do stuff
        }
        else
        {
            // nothing was chosen
        }
}

void DLWindow::SetConfiguration(Configuration *config)
{
    this->config = config;
}

DLWindow::~DLWindow()
{
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
        QMessageBox::warning(this, "Fehler beim Abruf", "Die Anfrage konnte nicht ausgeführt werden. \n\n Bitte probieren Sie es später erneut." ,QMessageBox::StandardButton::Ok, QMessageBox::StandardButton::Ok);
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

void DLWindow::populateListView()
{
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setRowCount(mp3s.count());

    ui->tableWidget->setColumnHidden(2, true); // hide url
    ui->tableWidget->setColumnHidden(4, true); // hide session


    QStringList labels("Interpret");
    labels.append("Titel");

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

        ui->tableWidget->setCellWidget(row, 3, qp);
        //qp->resize(cellwidth, 15);




        row++;
    }

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
    QString session = ui->tableWidget->item(row, 4)->text();

    qDebug() << "Starting download for row " << row << endl;


    for(int i = 0; i < mp3s.count(); i++) {
        if(mp3s.at(i)->Session == session){
           dlT->Mp3 = mp3s.at(i);
           dlT->progressBar = (QProgressBar *)ui->tableWidget->cellWidget(row, 3);
           dlT->DownloadDirectory = config->SaveDir;

           QThreadPool::globalInstance()->start(dlT);
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

    ui->tableWidget->selectAll();
    on_btnDownloadSelected_clicked();


}


void DLWindow::on_btnDownloadSelected_clicked()
{
    QList<QTableWidgetItem *> selectedItems = ui->tableWidget->selectedItems();
    foreach (QTableWidgetItem *item, selectedItems) {
        if(item->column() == 0) {
            startDownload(item->row());
        }
    }
}