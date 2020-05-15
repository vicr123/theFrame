#include "thememanager.h"

#include <tsettings.h>

struct ThemeManagerPrivate {
    tSettings settings;
    bool canUpdate = true;
};

ThemeManager::ThemeManager(QObject *parent) : QObject(parent)
{
    d = new ThemeManagerPrivate();

    connect(&d->settings, &tSettings::settingChanged, this, [=](QString key, QVariant value) {
        if (key == "Theme/mode") {
            updateTheme();
        }
    });
    updateTheme();
}

ThemeManager* ThemeManager::instance()
{
    static ThemeManager* instance = new ThemeManager();
    return instance;
}

void ThemeManager::updateTheme()
{
    if (!d->canUpdate) return;

    if (QStyleFactory::keys().contains("Contemporary")) {
        QPalette pal = QApplication::palette();

        if (d->settings.value("Theme/mode") == "light") {
            pal.setColor(QPalette::Button, QColor(0, 200, 255));
            pal.setColor(QPalette::ButtonText, QColor(0, 0, 0));
            pal.setColor(QPalette::Highlight, QColor(0, 175, 200));
            pal.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
            pal.setColor(QPalette::Disabled, QPalette::Button, QColor(0, 150, 200));
            pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(0, 0, 0));

            pal.setColor(QPalette::Window, QColor(210, 210, 210));
            pal.setColor(QPalette::Base, QColor(210, 210, 210));
            pal.setColor(QPalette::AlternateBase, QColor(210, 210, 210));
            pal.setColor(QPalette::WindowText, QColor(0, 0, 0));
            pal.setColor(QPalette::Text, QColor(0, 0, 0));
            pal.setColor(QPalette::ToolTipText, QColor(0, 0, 0));

            pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(100, 100, 100));
        } else {
            pal.setColor(QPalette::Button, QColor(0, 50, 150));
            pal.setColor(QPalette::ButtonText, QColor(255, 255, 255));
            pal.setColor(QPalette::Highlight, QColor(0, 80, 170));
            pal.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
            pal.setColor(QPalette::Disabled, QPalette::Button, QColor(0, 30, 100));
            pal.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(150, 150, 150));

            pal.setColor(QPalette::Window, QColor(40, 40, 40));
            pal.setColor(QPalette::Base, QColor(40, 40, 40));
            pal.setColor(QPalette::AlternateBase, QColor(60, 60, 60));
            pal.setColor(QPalette::WindowText, QColor(255, 255, 255));
            pal.setColor(QPalette::Text, QColor(255, 255, 255));
            pal.setColor(QPalette::ToolTipText, QColor(255, 255, 255));

            pal.setColor(QPalette::Disabled, QPalette::WindowText, QColor(150, 150, 150));
        }


        QApplication::setPalette(pal);
        QApplication::setPalette(pal, "QDockWidget");
        QApplication::setPalette(pal, "QToolBar");

        QApplication::setStyle("Contemporary");
    }

#ifdef Q_OS_MAC
    //For some reason macOS seems to hang when we change the theme
    //Require a restart for a theme change on macOS
    d->canUpdate = false;
#endif
}
