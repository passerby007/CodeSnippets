#-------------------------------------------------
#
# Project created by QtCreator 2015-04-04T22:18:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = JekyllPoster
TEMPLATE = app


SOURCES += main.cpp\
        MainDialog.cpp

HEADERS  += MainDialog.h

FORMS    += MainDialog.ui

OTHER_FILES += \
    gen_poster.py

RESOURCES +=

ICON = AppIcon.icns

# this is for MacOS/ios only, and should be resolved according to actually qt sdk path
QMAKE_INFO_PLIST = ../../libs/Qt/5.3/clang_64/mkspecs/macx-clang/Info.plist.app
