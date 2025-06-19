#include "CMainGUI.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Bootsmann");

    CMainGUI w;
    w.resize(1024,768);
    w.showMaximized();

    return a.exec();
}
