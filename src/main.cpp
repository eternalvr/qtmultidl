#include "dlwindow.h"
#include <QApplication>
#include "logindialog.h"

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   LoginDialog ld;
   DLWindow w;

   Configuration *c = new Configuration();

   ld.SetConfiguration(c);
   w.SetConfiguration(c);
   NetManager::GetInstance()->SetConfiguration(c);

   ld.Initialize();
   ld.exec();

   if(ld.result() == QDialog::DialogCode::Accepted){
       w.show();
       w.initialize();
       ld.close();
   } else {
       exit(0);
   }



    return a.exec();
}
