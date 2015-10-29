TEMPLATE = app

QT += qml quick widgets
QMAKE_CXXFLAGS += -Wall -Wextra -std=c++11

HEADERS +=
SOURCES += src/main.cpp

RESOURCES += resources/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(resources/deployment.pri)


