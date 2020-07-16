#-------------------------------------------------
#
# Project created by QtCreator 2020-07-04T23:37:47
#
#-------------------------------------------------

QT       += core gui sql network
QT += multimedia
QT += multimediawidgets
QT += charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SmartHosptal
TEMPLATE    = app
SOURCES     += \
    recommend.cpp \
    dbhelper.cpp \
    main.cpp
SOURCES     += smarthospital.cpp
HEADERS     += smarthospital.h \
    recommend.h \
    dbhelper.h
FORMS       += smarthospital.ui \
    recommend.ui

INCLUDEPATH += $$PWD/sdk

CONFIG(debug, debug|release){
LIBS += -L$$PWD/sdk/ -lqucd
} else {
LIBS += -L$$PWD/sdk/ -lquc
}
CONFIG(c++11)

RESOURCES += \
    resource.qrc













