/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2020 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "mainwindow.h"

#include <tapplication.h>
#include <QDir>
#include <QIcon>
#include <QStyleFactory>
#include <tsettings.h>
#include "managers/thememanager.h"

#ifdef Q_OS_MAC
    extern void setupMacObjC();
#endif

int main(int argc, char* argv[]) {
//    qputenv("QT_FONT_DPI", "96");
    tApplication a(argc, argv);

    if (QDir("/usr/share/theframe").exists()) {
        a.setShareDir("/usr/share/theframe");
    } else if (QDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/theframe/")).exists()) {
        a.setShareDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../share/theframe/"));
    }
    a.installTranslators();

    //Install libtheframe translators
    QTranslator* libtheframeTranslator = new QTranslator();
#if defined(Q_OS_MAC)
    libtheframeTranslator->load(QLocale::system().name(), a.macOSBundlePath() + "/Contents/Frameworks/libtheframe.framework/Contents/translations/");
#elif defined(Q_OS_LINUX)
    libtheframeTranslator->load(QLocale::system().name(), a.shareDir() + "/libtheframe/translations");
#elif defined(Q_OS_WIN)
    libtheframeTranslator->load(QLocale::system().name(), a.applicationDirPath() + "\\libtheframe\\translations");
#endif
    a.installTranslator(libtheframeTranslator);

    a.setOrganizationName("theSuite");
    a.setOrganizationDomain("vicr123.com");
    a.setApplicationName("theFrame");
    a.setDesktopFileName("com.vicr123.theframe");

#if defined(Q_OS_WIN)
    QIcon::setThemeName("contemporary-icons");
    QIcon::setThemeSearchPaths({a.applicationDirPath() + "\\icons"});

    tSettings::registerDefaults(a.applicationDirPath() + "/defaults.conf");
#elif defined(Q_OS_MAC)
    QIcon::setThemeName("contemporary-icons");
    QIcon::setThemeSearchPaths({a.macOSBundlePath() + "/Contents/Resources/icons"});

    tSettings::registerDefaults(a.macOSBundlePath() + "/Contents/Resources/defaults.conf");

    a.setQuitOnLastWindowClosed(false);
    setupMacObjC();
#else
    if (QDir("/etc/theframe/defaults.conf").exists()) {
        tSettings::registerDefaults("/etc/theframe/defaults.conf");
    } else if (QDir(QDir::cleanPath(QApplication::applicationDirPath() + "/../etc/theframe/defaults.conf")).exists()) {
        tSettings::registerDefaults(QDir::cleanPath(QApplication::applicationDirPath() + "/../etc/theframe/defaults.conf"));
    }
#endif

    //Initialise the theme manager
    ThemeManager::instance();

    a.setApplicationIcon(QIcon::fromTheme("theframe", QIcon(":/icons/theframe.svg")));
    a.setAboutDialogSplashGraphic(a.aboutDialogSplashGraphicFromSvg(":/icons/aboutsplash.svg"));
    a.setApplicationLicense(tApplication::Gpl3OrLater);
    a.setCopyrightHolder("Victor Tran");
    a.setCopyrightYear("2020");
    a.setApplicationVersion("1.0");
    a.setGenericName(QApplication::translate("main", "Animation Tool"));

    QCommandLineParser parser;
    parser.addPositionalArgument("files", a.translate("main", "Project files to open"), "files");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(a);

    bool openedFirstFile = false;

    if (parser.positionalArguments().isEmpty()) {
        MainWindow* w = new MainWindow();
        w->show();

        QObject::connect(&a, &tApplication::openFile, [ =, &openedFirstFile](QString file) {
            MainWindow* window;
            if (openedFirstFile) {
                window = new MainWindow();
            } else {
                window = w;
            }
            window->openFile(file);
            openedFirstFile = true;
        });
    } else {
        for (QString arg : parser.positionalArguments()) {
            QString fileName = arg;
            if (QUrl(fileName).isValid()) fileName = QUrl(fileName).toLocalFile();
            MainWindow* w = new MainWindow();
            w->openFile(fileName);
            w->show();
        }
    }

    return a.exec();
}
