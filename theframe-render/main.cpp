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
#include <QGuiApplication>
#include "renderer.h"

static Renderer renderer;

#ifdef Q_OS_UNIX
#include <signal.h>

void signalHandler(int signal) {
    renderer.stop();
}

void prepareTraps() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGHUP);
    sigaddset(&mask, SIGTERM);

    struct sigaction action;
    action.sa_handler = signalHandler;
    action.sa_mask = mask;
    action.sa_flags = 0;

    sigaction(SIGINT, &action, nullptr);
    sigaction(SIGHUP, &action, nullptr);
    sigaction(SIGTERM, &action, nullptr);
}

#else
void prepareTraps() {

}
#endif

int main(int argc, char* argv[]) {
#ifdef Q_OS_LINUX
    qputenv("QT_QPA_PLATFORM", "offscreen");
#endif
    QGuiApplication a(argc, argv);

    //Catch signals on Linux
    prepareTraps();

    if (!renderer.prepare()) return 1;
    if (!renderer.constructElements()) return 2;

    renderer.beginRender();
    if (renderer.isStopped()) return 0;

    if (!renderer.completeRender()) return 3;

    return 0;
}
