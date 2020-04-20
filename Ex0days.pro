QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ex0days
TEMPLATE = app

CONFIG += c++14

win32: {
    RC_ICONS += ex0days.ico

# we need the console to be able to print stuff in command line mode...
# we hide the console if we start in GUI mode
    CONFIG += console
}



CONFIG(debug, debug|release) : {
    DEFINES += __DEBUG__
}
else {
    # In release mode, remove all qDebugs !
    DEFINES += QT_NO_DEBUG_OUTPUT
}

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    About.cpp \
    CmdOrGuiApp.cpp \
    Ex0days.cpp \
    SignedListWidget.cpp \
    main.cpp \
    MainWindow.cpp

HEADERS += \
    About.h \
    CmdOrGuiApp.h \
    Ex0days.h \
    MainWindow.h \
    SignedListWidget.h

FORMS += \
    About.ui \
    MainWindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
