#include "mainwindow.h"

#include <QApplication>






int main(int argc, char *argv[])
{
    carData car;
    carConfig conf;
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
