#include <QApplication>
#include "KCPresta.h"
#include "KCFirma.h"
#include "Logger.h"
#include <iostream>
#include <QSettings>
#include "MainWindow.h"

#include <QDebug>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow main;
    main.show();

    return a.exec();
}
