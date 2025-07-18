QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
QT += multimedia multimediawidgets

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    audiohandler.cpp \
    effectmanager.cpp \
    gamescene.cpp \
    main.cpp \
    mainwindow.cpp \
    menuUI.cpp \
    note.cpp \
    notemanager.cpp \
    particle.cpp

HEADERS += \
    audiohandler.h \
    effectmanager.h \
    gamescene.h \
    mainwindow.h \
    menuUI.h \
    note.h \
    notemanager.h \
    particle.h

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc
