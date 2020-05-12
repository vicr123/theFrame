QT       += core gui concurrent network
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
    properties/anchorproperty.cpp \
    properties/colorproperty.cpp \
    properties/fileproperty.cpp \
    properties/floatproperty.cpp \
    properties/fontproperty.cpp \
    properties/percentageproperty.cpp \
    properties/pointproperty.cpp \
    properties/propertywidget.cpp \
    properties/rectproperty.cpp \
    properties/stringproperty.cpp \
    propertieswidget.cpp \
    render/ffmpegdetector.cpp \
    render/rendercontroller.cpp \
    render/renderjob.cpp \
    render/renderjobs.cpp \
    render/renderjobwidget.cpp \
    render/renderpopover.cpp \
    settings/advancedsettings.cpp \
    settings/generalsettings.cpp \
    settings/settingsdialog.cpp \
    settings/tutorialssettings.cpp \
    timeline/timeline.cpp \
    timeline/timelineleftwidget.cpp \
    timeline/timelinerightwidget.cpp \
    timeline/timelinerightwidgetproperty.cpp \
    tutorialengine.cpp \
    tutorialwindow.cpp \
    undo/elementstate.cpp \
    undo/timelineelementstate.cpp \
    undo/undodeleteelement.cpp \
    undo/undodeletetimelineelement.cpp \
    undo/undoelementmodify.cpp \
    undo/undonewelement.cpp \
    undo/undonewtimelineelement.cpp \
    undo/undotimelineelementmodify.cpp \
    viewport.cpp \
    viewport/pointviewport.cpp \
    viewport/rectangleviewport.cpp \
    viewport/viewportproperty.cpp

HEADERS += \
    mainwindow.h \
    prerenderer.h \
    properties/anchorproperty.h \
    properties/colorproperty.h \
    properties/fileproperty.h \
    properties/floatproperty.h \
    properties/fontproperty.h \
    properties/percentageproperty.h \
    properties/pointproperty.h \
    properties/propertywidget.h \
    properties/rectproperty.h \
    properties/stringproperty.h \
    propertieswidget.h \
    render/ffmpegdetector.h \
    render/rendercontroller.h \
    render/renderjob.h \
    render/renderjobs.h \
    render/renderjobwidget.h \
    render/renderpopover.h \
    settings/advancedsettings.h \
    settings/generalsettings.h \
    settings/settingsdialog.h \
    settings/tutorialssettings.h \
    timeline/timeline.h \
    timeline/timelineleftwidget.h \
    timeline/timelinerightwidget.h \
    timeline/timelinerightwidgetproperty.h \
    tutorialengine.h \
    tutorialwindow.h \
    undo/elementstate.h \
    undo/timelineelementstate.h \
    undo/undodeleteelement.h \
    undo/undodeletetimelineelement.h \
    undo/undoelementmodify.h \
    undo/undonewelement.h \
    undo/undonewtimelineelement.h \
    undo/undotimelineelementmodify.h \
    viewport.h \
    viewport/pointviewport.h \
    viewport/rectangleviewport.h \
    viewport/viewportproperty.h

FORMS += \
    mainwindow.ui \
    properties/anchorproperty.ui \
    properties/colorproperty.ui \
    properties/fileproperty.ui \
    properties/floatproperty.ui \
    properties/fontproperty.ui \
    properties/percentageproperty.ui \
    properties/pointproperty.ui \
    properties/rectproperty.ui \
    properties/stringproperty.ui \
    propertieswidget.ui \
    render/renderjobs.ui \
    render/renderjobwidget.ui \
    render/renderpopover.ui \
    settings/advancedsettings.ui \
    settings/generalsettings.ui \
    settings/settingsdialog.ui \
    settings/tutorialssettings.ui \
    timeline/timeline.ui \
    timeline/timelineleftwidget.ui \
    timeline/timelinerightwidget.ui \
    tutorialwindow.ui \
    viewport.ui

unix:!macx {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    QT += thelib
    LIBS += -L$$OUT_PWD/../libtheframe/ -ltheframe

    TARGET = theframe
    target.path = /usr/bin

    appentry.path = /usr/share/applications
    appentry.files = com.vicr123.theframe.desktop

    icon.path = /usr/share/icons/hicolor/scalable/apps/
    icon.files = theframe.svg

    translations.path = /usr/share/theframe/translations
    translations.files = translations/*

    defaults.path = /etc/theframe/
    defaults.files = defaults.conf

    mime.path = /usr/share/mime/packages/
    mime.files = application-x-theframe-project.xml

    INSTALLS += target appentry icon translations defaults mime
}

win32 {
    # Include the-libs build tools
    include(C:/Program Files/thelibs/pri/buildmaster.pri)

    CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../libtheframe/release/ -llibtheframe
    CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../libtheframe/debug/ -llibtheframe

    INCLUDEPATH += "C:/Program Files/thelibs/include"
    LIBS += -L"C:/Program Files/thelibs/lib" -lthe-libs
    RC_FILE = icon.rc
    TARGET = theFrame
}

macx {
    # Include the-libs build tools
    include(/usr/local/share/the-libs/pri/buildmaster.pri)

    QT += macextras
    LIBS += -framework CoreFoundation -framework AppKit

    SOURCES += \
        main-objc.mm \
        mainwindow-objc.mm \
        render/renderpopover-objc.mm

    blueprint {
        TARGET = "theFrame Blueprint"
        ICON = icon-bp.icns
    } else {
        TARGET = "theFrame"
        ICON = icon.icns
    }

    INCLUDEPATH += "/usr/local/include/the-libs"
    LIBS += -L/usr/local/lib -lthe-libs -F$$OUT_PWD/../libtheframe/ -framework libtheframe

    QMAKE_POST_LINK += $$quote(cp $${PWD}/dmgicon.icns $${PWD}/app-dmg-background.png $${PWD}/node-appdmg-config*.json $${OUT_PWD}/..)

    icons.path = Contents/Resources/icons
    icons.files = icons/contemporary-icons

    defaults.path = Contents/Resources/
    defaults.files = defaults.conf

    QMAKE_BUNDLE_DATA += icons defaults
    QMAKE_INFO_PLIST = Info.plist
}

INCLUDEPATH += $$PWD/../libtheframe
DEPENDPATH += $$PWD/../libtheframe

DISTFILES += \
    Info.plist \
    application-x-theframe-project.xml \
    com.vicr123.theframe.desktop \
    defaults.conf

RESOURCES += \
    resources.qrc
