QT       += core gui thelib
SHARE_APP_NAME = theframe

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

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
    main.cpp \
    mainwindow.cpp \
    prerenderer.cpp \
    properties/floatproperty.cpp \
    properties/fontproperty.cpp \
    properties/percentageproperty.cpp \
    properties/pointproperty.cpp \
    properties/propertywidget.cpp \
    properties/rectproperty.cpp \
    properties/stringproperty.cpp \
    propertieswidget.cpp \
    timeline/timeline.cpp \
    timeline/timelineleftwidget.cpp \
    timeline/timelinerightwidget.cpp \
    timeline/timelinerightwidgetproperty.cpp \
    viewport.cpp

HEADERS += \
    mainwindow.h \
    prerenderer.h \
    properties/floatproperty.h \
    properties/fontproperty.h \
    properties/percentageproperty.h \
    properties/pointproperty.h \
    properties/propertywidget.h \
    properties/rectproperty.h \
    properties/stringproperty.h \
    propertieswidget.h \
    timeline/timeline.h \
    timeline/timelineleftwidget.h \
    timeline/timelinerightwidget.h \
    timeline/timelinerightwidgetproperty.h \
    viewport.h

FORMS += \
    mainwindow.ui \
    properties/floatproperty.ui \
    properties/fontproperty.ui \
    properties/percentageproperty.ui \
    properties/pointproperty.ui \
    properties/rectproperty.ui \
    properties/stringproperty.ui \
    propertieswidget.ui \
    timeline/timeline.ui \
    timeline/timelineleftwidget.ui \
    timeline/timelinerightwidget.ui \
    viewport.ui

unix:!macx {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    TARGET = theframe
    target.path = /usr/bin

    appentry.path = /usr/share/applications
    appentry.files = com.vicr123.theframe.desktop

    icon.path = /usr/share/icons/hicolor/scalable/apps/
    icon.files = theframe.svg

    translations.path = /usr/share/theframe/translations
    translations.files = translations/*

    INSTALLS += target appentry icon translations
}

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libtheframe/release/ -ltheframe
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libtheframe/debug/ -ltheframe
else:mac: LIBS += -F$$OUT_PWD/../libtheframe/ -framework theframe
else:unix: LIBS += -L$$OUT_PWD/../libtheframe/ -ltheframe

INCLUDEPATH += $$PWD/../libtheframe
DEPENDPATH += $$PWD/../libtheframe

DISTFILES += \
    com.vicr123.theframe.desktop
