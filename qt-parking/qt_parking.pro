QT += core gui widgets serialport multimedia

CONFIG += c++17 warn_on
TEMPLATE = app
TARGET = qt_parking

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    loadingdialog.cpp \
    entrydialog.cpp \
    exitdialog.cpp \
    vehiclelistdialog.cpp \
    serialcontroller.cpp \
    cameraview.cpp \
    parkingstore.cpp

HEADERS += \
    mainwindow.h \
    loadingdialog.h \
    entrydialog.h \
    exitdialog.h \
    vehiclelistdialog.h \
    serialcontroller.h \
    cameraview.h \
    parkingstore.h

FORMS += \
    mainwindow.ui \
    loadingdialog.ui \
    entrydialog.ui \
    exitdialog.ui \
    vehiclelistdialog.ui
