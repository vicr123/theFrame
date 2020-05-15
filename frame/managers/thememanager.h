#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>

struct ThemeManagerPrivate;
class ThemeManager : public QObject
{
        Q_OBJECT
    public:
        explicit ThemeManager(QObject *parent = nullptr);

        static ThemeManager* instance();

    signals:

    private:
        ThemeManagerPrivate* d;

        void updateTheme();
};

#endif // THEMEMANAGER_H
