#include "mainwindow.h"

#include <QApplication>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // declared as static to allow more memory reservation
    static MainWindow w(nullptr);
    w.show();
    return a.exec();
}
