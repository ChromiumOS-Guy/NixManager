/*
 * This file is part of system-settings
 *
 * Copyright (C) 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * made by ChromiumOS-Guy (https://github.com/ChromiumOS-Guy)
 */

#ifndef WORKER_LOGIC_H
#define WORKER_LOGIC_H

#include <QStringList>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
// custom libs
#include "nix-layer/nix-wrapper.h"
#include "nix-setup.h"

/**
 * @brief The WorkerLogic class contains the direct, synchronous calls
 * to the Nix C++ wrapper library.
 * This class MUST NOT inherit from QObject and MUST NOT contain signals/slots,
 * as its methods are designed to be called only from the separate QThread's Worker object.
 */
class WorkerLogic
{
public:
    WorkerLogic() = delete; // Utility class, prevent instantiation

    // =========================================================================
    // Blocking Call Wrappers (Return JSON string directly)
    // =========================================================================

    static QString hm_switch_sync(const bool allow_insecure);
    static QString hm_version_sync();
    static QString read_packages_sync(const QString& packageType);
    static QString add_packages_sync(const QString& packagesJsonString, bool allow_insecure, const QString& packageType, bool overwrite);
    static QString delete_packages_sync(const QString& packagesJsonString, const QString& packageType);
    static QString search_packages_sync(const QString& quarry, const bool local, const QString& base_url, const int timeout);
    static QString update_channels_sync();
    static QString list_channels_sync();
    static QString add_channel_sync(const QString& url, const QString& name);
    static QString remove_channel_sync(const QString& name);
    static QString list_generations_sync();
    static QString switch_generation_sync(const QString& generation_id);
    static QString delete_generation_sync(const QString& generation_id);
    static QString delete_old_generations_sync();
    static QString hm_expire_generations_sync(const QString& timestamp);
    static QString hm_list_generations_sync();

    // =========================================================================
    // Blocking Setup Wrappers (Return custom formatted JSON string)
    // =========================================================================
    static QString install_nix_home_manager_sync(const QString& version);
    static QString uninstall_nix_home_manager_sync();
    static QString detect_nix_home_manager_sync();
};

#endif // WORKER_LOGIC_H