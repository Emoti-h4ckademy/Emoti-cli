#include <QApplication>

#include <QQmlApplicationEngine>
#include <QSplashScreen>
#include <QStyle>
#include <QDesktopWidget>

#include "mainwindow.h"

#include <QTime>
void delay()
{
    QTime dieTime= QTime::currentTime().addSecs(2);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1000);
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(emoti);

    QWidget *myWindow;

    QApplication app(argc, argv);
    app.setOrganizationName("H4ckademy");
    app.setApplicationName("Emoti client");



    QPixmap pixmap(":/resources/emoti-logo-splash.png");
    myWindow = new QSplashScreen(pixmap);
    myWindow->show();
    reinterpret_cast<QSplashScreen*> (myWindow)->showMessage("Hello Emoti", Qt::AlignHCenter | Qt::AlignBottom , Qt::black);

    delay(); ////---- Simulation: creating everything that's needed
    delete(myWindow);


    myWindow = new MainWindow();
    myWindow->setGeometry(
                QStyle::alignedRect(
                    Qt::LeftToRight,
                    Qt::AlignCenter,
                    myWindow->size(),
                    app.desktop()->availableGeometry()
                    )
                );

    myWindow->show();

    return app.exec();
}
