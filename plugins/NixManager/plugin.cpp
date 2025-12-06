/*
 * Copyright (C) 2025  Your FullName
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * nixmanager is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtQml>
#include <QtQml/QQmlContext>

#include "plugin.h"
#include "controller.h"

void NixManagerPlugin::registerTypes(const char *uri) {
    //@uri NixManager
    qmlRegisterSingletonType<Controller>(uri, 1, 0, "NixManagerPlugin", [](QQmlEngine*, QJSEngine*) -> QObject* { return new Controller; });
}
