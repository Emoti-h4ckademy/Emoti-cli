#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    //Q_INIT_RESOURCE(application);

    QApplication app(argc, argv);
    app.setOrganizationName("H4ckademy");
    app.setApplicationName("Emoti client");
    MainWindow mainWin;
    mainWin.show();
    return app.exec();
}
