TEMPLATE = app

QT += qml quick widgets network
QMAKE_CXXFLAGS += -Wall -Wextra -std=c++11

CONFIG += link_pkgconfig
PKGCONFIG += opencv

HEADERS += \
    src/camera.h \
    src/network.h
SOURCES += src/main.cpp \
    src/camera.cpp \
    src/network.cpp

RESOURCES += resources/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(resources/deployment.pri)
