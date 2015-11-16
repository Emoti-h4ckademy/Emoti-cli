TEMPLATE = app

QT += widgets network multimedia qml quick
QMAKE_CXXFLAGS += -Wall -Wextra
CONFIG += c++11

HEADERS += \
    src/camera.h \
    src/network.h \
    src/camimage.h \
    src/mainwindow.h
SOURCES += src/main.cpp \
    src/camera.cpp \
    src/network.cpp \
    src/camimage.cpp \
    src/mainwindow.cpp

RESOURCES += \
    resources/emoti/emoti.qrc

CONFIG(release, debug|release) {
    #This is a release build
    DEFINES += QT_NO_DEBUG_OUTPUT
} else {
    #This is a debug build
}

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(resources/deployment.pri)

FORMS += \
    src/mainwindow.ui

DISTFILES +=
