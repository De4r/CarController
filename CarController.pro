QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
LIBS += -lpthread -lrt -lpigpiod_if2
INCLUDEPATH += /raspi/sysroot/usr/include/
SOURCES += \
    common.cpp \
    ifrsensor.cpp \
    joypad.cpp \
    main.cpp \
    mainwindow.cpp \
    motorsteering.cpp \
    sma.cpp \
    sonar.cpp

HEADERS += \
    common.h \
    ifrsensor.h \
    joypad.h \
    mainwindow.h \
    motorsteering.h \
    sma.h \
    sonar.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.

qnx: target.path = /home/pi/Desktop
else: unix:!android: target.path = /home/pi/Desktop
!isEmpty(target.path): INSTALLS += target
