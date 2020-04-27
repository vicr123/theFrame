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
#ifndef RENDERER_H
#define RENDERER_H

#include <QObject>

struct RendererPrivate;
class Renderer : public QObject {
        Q_OBJECT
    public:
        explicit Renderer(QObject* parent = nullptr);
        ~Renderer();

        bool prepare();
        bool constructElements();

        void beginRender();
        bool completeRender();

        void stop();
        bool isStopped();

    signals:
        void stopProgress();

    private:
        RendererPrivate* d;

        void logError(QString error);
        void logInfo(QString info);
        void logProgress(QString progress);
};

#endif // RENDERER_H
