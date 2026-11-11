#include "mainwindow.h"
#include "DatabaseManager.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DatabaseManager dbManager;
    if(!dbManager.connect())
        return -1;
    if(!dbManager.initSchema())
        return -1;
    MainWindow w;
    w.show();
    return QApplication::exec();
}
