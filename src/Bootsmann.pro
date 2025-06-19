QT += core gui network
QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += $$files(*.cpp)
HEADERS += $$files(*.h)
FORMS += $$files(*.ui)
RESOURCES += $$files(*.qrc)

# QHexView
HEADERS += $$PWD/QHexView/include/QHexView.h
SOURCES += $$PWD/QHexView/src/QHexView.cpp
INCLUDEPATH += $$PWD/QHexView/include

# QSourceHighlite
include($$PWD/QSourceHighlite/QSourceHighlite.pri)
INCLUDEPATH += $$PWD/QSourceHighlite

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32{
	VERSION = 0.1.0.0
	RC_ICONS = $$PWD/../win32/icon.ico
	QMAKE_TARGET_COPYRIGHT = (C) 2025 Ars L. Masiuk
	QMAKE_TARGET_DESCRIPTION = Bootsmann REST API Tester
	QMAKE_TARGET_PRODUCT = Bootsmann
	LIBS += -ladvapi32
}

