TEMPLATE = app

QT += qml quick widgets network multimedia
QMAKE_CXXFLAGS += -Wall -Wextra
CONFIG += c++11

HEADERS += \
    src/camera.h \
    src/network.h \
    src/camimage.h
SOURCES += src/main.cpp \
    src/camera.cpp \
    src/network.cpp \
    src/camimage.cpp

RESOURCES += resources/qml.qrc

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
