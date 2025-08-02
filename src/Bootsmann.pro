QT += core gui network
QT += widgets
QT += webenginewidgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += $$files(*.cpp)
HEADERS += $$files(*.h)
FORMS += $$files(*.ui)
RESOURCES += $$files(*.qrc)

# QHexView
#HEADERS += $$PWD/QHexView/include/QHexView.h
#SOURCES += $$PWD/QHexView/src/QHexView.cpp
#INCLUDEPATH += $$PWD/QHexView/include

# QHexView2
INCLUDEPATH += $$PWD/QHexView2
HEADERS += $$PWD/QHexView2/qhexview.h
SOURCES += $$PWD/QHexView2/qhexview.cpp

# QSourceHighlite
include($$PWD/QSourceHighlite/QSourceHighlite.pri)
INCLUDEPATH += $$PWD/QSourceHighlite

# QCodeEditor
INCLUDEPATH += $$PWD/QCodeEditor/include
HEADERS += $$files($$PWD/QCodeEditor/include/internal/*.hpp)
SOURCES += $$files($$PWD/QCodeEditor/src/internal/*.cpp)
RESOURCES += $$files($$PWD/QCodeEditor/resources/*.qrc)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32{
	VERSION = 0.1.1.0
	RC_ICONS = $$PWD/../win32/icon.ico
	QMAKE_TARGET_COPYRIGHT = (C) 2025 BootsmannApp
	QMAKE_TARGET_DESCRIPTION = Bootsmann REST API Testing Software
	QMAKE_TARGET_PRODUCT = Bootsmann
	LIBS += -ladvapi32

        win32-msvc: QMAKE_CXXFLAGS += /MP
}

