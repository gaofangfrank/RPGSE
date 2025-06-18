QT_MAJOR_VERSION = 5
QT_MINOR_VERSION = 15

INCLUDEPATH += $$PWD/include $$PWD/qt-lzstring/src
DEPENDPATH += $$PWD/include $$PWD/qt-lzstring/src

QMAKE_CC = clang
QMAKE_CXX = clang++
QMAKE_LINK = clang++
QMAKE_LFLAGS = -fuse-ld=lld

CONFIG += debug_and_release C++17 static

QT += widgets

HEADERS += $$PWD/qt-lzstring/src/lzstring.h $$PWD/include/MainWindow.h $$PWD/include/SaveElements.h
SOURCES += qt-lzstring/src/lzstring.cpp lib/MainWindow.cpp lib/main.cpp lib/SaveElements.cpp
