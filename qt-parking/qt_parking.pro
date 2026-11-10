QT += core gui widgets serialport sql

qtHaveModule(multimedia) {
    QT += multimedia
    DEFINES += PARKING_WITH_CAMERA
}

CONFIG += c++17 warn_on
CONFIG += link_pkgconfig
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
    camerathread.cpp \
    cardbmanager.cpp \
    parkingstore.cpp

HEADERS += \
    mainwindow.h \
    loadingdialog.h \
    entrydialog.h \
    exitdialog.h \
    vehiclelistdialog.h \
    serialcontroller.h \
    camerathread.h \
    cardbmanager.h \
    parkingstore.h

FORMS += \
    mainwindow.ui \
    loadingdialog.ui \
    entrydialog.ui \
    exitdialog.ui \
    vehiclelistdialog.ui

# The requested MSYS2 package used opencv4 until OpenCV 5; current packages
# expose opencv5.pc, so keep the requested name first and fall back cleanly.
packagesExist(opencv4) {
    PKGCONFIG += opencv4
    DEFINES += PARKING_WITH_OPENCV
} else:packagesExist(opencv5) {
    PKGCONFIG += opencv5
    DEFINES += PARKING_WITH_OPENCV
} else {
    error(OpenCV pkg-config package not found)
}
