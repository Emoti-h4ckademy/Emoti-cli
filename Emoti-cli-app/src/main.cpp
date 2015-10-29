#include <QApplication>
#include <QQmlApplicationEngine>
#include <QMessageBox>
#include <QAction>
#include <QMenu>
#include <QSystemTrayIcon>

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


int loadTray(QMenu *_trayIconMenu, QSystemTrayIcon *_trayIcon, QQmlApplicationEngine *engine)
{

    QObject *root = engine->rootObjects().at(0);

    QAction *minimizeAction = new QAction(QObject::tr("Mi&nimize"), root);
    root->connect(minimizeAction, SIGNAL(triggered()), root, SLOT(hide()));
    QAction *maximizeAction = new QAction(QObject::tr("Ma&ximize"), root);
    root->connect(maximizeAction, SIGNAL(triggered()), root, SLOT(showMaximized()));
    QAction *restoreAction = new QAction(QObject::tr("&Restore"), root);
    root->connect(restoreAction, SIGNAL(triggered()), root, SLOT(showNormal()));
    QAction *quitAction = new QAction(QObject::tr("&Quit"), root);
    root->connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    _trayIconMenu = new QMenu();
    _trayIconMenu->addAction(minimizeAction);
    _trayIconMenu->addAction(maximizeAction);
    _trayIconMenu->addAction(restoreAction);
    _trayIconMenu->addSeparator();
    _trayIconMenu->addAction(quitAction);

    _trayIcon = new QSystemTrayIcon(root);
    _trayIcon->setContextMenu(_trayIconMenu);
    _trayIcon->setIcon(QIcon(":/resources/emoti-icon.png"));
    _trayIcon->show();

    return 0;

}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);

    QQmlApplicationEngine *engine = loadTrayResources();
    if (engine == nullptr)
    {
        return 1;
    }

    if (engine->rootObjects().size() == 0)
    {
        QObject::tr("Could not load resources");
        return 1;
    }


    QMenu *trayIconMenu = nullptr;
    QSystemTrayIcon *trayIcon = nullptr;

    if (loadTray(trayIconMenu,trayIcon, engine) != 0)
    {
        return 1;
    }

    return app.exec();
}
