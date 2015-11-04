#include <QApplication>
#include <QQmlApplicationEngine>
#include <QMessageBox>
#include <QAction>
#include <QMenu>
#include <QSystemTrayIcon>

#include <QDebug>
#include <QTime>

#include "camera.h"
#include "network.h"

QQmlApplicationEngine* loadTrayResources()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        QMessageBox::critical(0, QObject::tr("Systray"),
                                 QObject::tr("I couldn't detect any system tray "
                                             "on this system."));
        return nullptr;
    }

    QQmlApplicationEngine *engine = new QQmlApplicationEngine();
    engine->load(QUrl(QStringLiteral("qrc:///resources/main.qml")));

    return engine;
}


int loadTray(QSystemTrayIcon *_trayIcon, QQmlApplicationEngine *engine)
{

    QObject *root = engine->rootObjects().at(0);

//    QAction *minimizeAction = new QAction(QObject::tr("Mi&nimize"), root);
//    root->connect(minimizeAction, SIGNAL(triggered()), root, SLOT(hide()));

//    QAction *maximizeAction = new QAction(QObject::tr("Ma&ximize"), root);
//    root->connect(maximizeAction, SIGNAL(triggered()), root, SLOT(showMaximized()));

    QAction *restoreAction = new QAction(QObject::tr("&Restore"), root);
    root->connect(restoreAction, SIGNAL(triggered()), root, SLOT(showNormal()));

    QAction *quitAction = new QAction(QObject::tr("&Quit"), root);
    root->connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));


//    _trayIcon->contextMenu()->addAction(minimizeAction);
//    _trayIcon->contextMenu()->addAction(maximizeAction);


    _trayIcon->contextMenu()->addAction(restoreAction);
    _trayIcon->contextMenu()->addAction(quitAction);
//    _trayIconMenu->addAction(restoreAction);
//    _trayIconMenu->addAction(quitAction);


    _trayIcon->setParent(root);
    _trayIcon->setIcon(QIcon(":/resources/emoti-icon.png"));
    _trayIcon->show();

    return 0;

}

int main(int argc, char *argv[])
{
//    QApplication app(argc, argv);
//    QApplication::setQuitOnLastWindowClosed(false);

//    std::shared_ptr<QQmlApplicationEngine> engine (loadTrayResources());
//    if ((engine == nullptr) || (engine->rootObjects().size() == 0))
//    {
//        fprintf(stderr, "Could not load resources");
//        return 1;
//    }


//    std::shared_ptr<QMenu> trayIconMenu (new QMenu());
//    std::shared_ptr<QSystemTrayIcon> trayIcon (new QSystemTrayIcon());
//    trayIcon.get()->setContextMenu(trayIconMenu.get());

//    if (loadTray(trayIcon.get(), engine.get()) != 0)
//    {
//        return 1;
//    }


    Camera cam;
    cam.initCamera(0);

    std::shared_ptr<PngImage> test = cam.getImage();

    QString name = qgetenv("USER");
    if (name.isEmpty())
        name = qgetenv("USERNAME");

    QDateTime time = QDateTime::currentDateTime();
    Network net("http://10.102.83.80:8080/imagepost");


    net.sendImage(test, name, time.toString());



//    FILE *fl = fopen("salida.png", "w+");
//    if (fl == NULL){
//        fprintf(stderr,"Couldn't open destiny location\n");
//        return -1;
//    }

//    fwrite(test.get()->getData().get(),sizeof(char), test.get()->getSize(),fl);

//    fclose(fl);


//    app.exec();

}
