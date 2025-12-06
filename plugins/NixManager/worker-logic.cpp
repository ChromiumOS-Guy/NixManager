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


#include "worker-logic.h" // Keep this here at the top


// read_packages /add_packages /delete_packages/ update_packages output:
// // On Success
// {
//   "success": true, // bool
//   "message": "Successfully did something.", // QString::fromStdString
//   "output": [ "package1", "package2" ], // stringVectorToJsonArray
//   "simple_error": QJsonArray(),
//   "full_error": QJsonArray()
// }

// // On Failure (due to apply_config errors)
// {
//   "success": false, // bool
//   "message": "Failed to apply changes, backup restored.", // QString::fromStdString
//   "output": QJsonArray(),
//   "simple_error": [ "simple_error_msg1" ], // stringVectorToJsonArray
//   "full_error": [ "full_error_line1", "full_error_line2" ] //stringVectorToJsonArray
// }

// // On Failure (due to internal errors like JSON parsing or backup/restore)
// {
//   "success": false, // bool
//   "message": "A critical error occurred.", // QString::fromStdString
//   "output": QJsonArray(),
//   "simple_error": [ "User-friendly message about the internal error" ], // stringVectorToJsonArray
//   "full_error": [ "Detailed technical error message, e.g., exception details" ] // stringVectorToJsonArray
// }

// nix-layer API for QT GUI.
QString WorkerLogic::hm_switch_sync(const bool allow_insecure)
{
    return PackageManipulation::hm_switch_wrapper(allow_insecure);
}

QString WorkerLogic::hm_version_sync()
{
    return PackageManipulation::hm_version_wrapper();
}

QString WorkerLogic::read_packages_sync(const QString& packageType)
{
    return PackageManipulation::read_packages_wrapper(packageType);
}

QString WorkerLogic::add_packages_sync(const QString& packagesJsonString, bool allow_insecure, const QString& packageType, bool overwrite)
{
    return PackageManipulation::add_packages_wrapper(packagesJsonString, allow_insecure, packageType, overwrite);
}

QString WorkerLogic::delete_packages_sync(const QString& packagesJsonString, const QString& packageType)
{
    return PackageManipulation::delete_packages_wrapper(packagesJsonString, packageType);
}

QString WorkerLogic::search_packages_sync(const QString& quarry, const bool local, const QString& base_url, const int timeout)
{
    return PackageManipulation::search_packages_wrapper(quarry, local, base_url, timeout);
}

QString WorkerLogic::update_channels_sync()
{
    return ChannelManipulation::update_channels_wrapper();
}

QString WorkerLogic::list_channels_sync()
{
    return ChannelManipulation::list_channels_wrapper();
}

QString WorkerLogic::add_channel_sync(const QString& url, const QString& name)
{
    return ChannelManipulation::add_channels_wrapper(url, name);
}

QString WorkerLogic::remove_channel_sync(const QString& name)
{
    return ChannelManipulation::remove_channels_wrapper(name);
}

QString WorkerLogic::list_generations_sync()
{
    return GenerationManipulation::list_generations_wrapper();
}

QString WorkerLogic::switch_generation_sync(const QString& generation_id)
{
    return GenerationManipulation::switch_generation_wrapper(generation_id);
}

QString WorkerLogic::delete_generation_sync(const QString& generation_id)
{
    return GenerationManipulation::delete_generation_wrapper(generation_id);
}

QString WorkerLogic::delete_old_generations_sync()
{
    return GenerationManipulation::delete_old_generations_wrapper();
}

QString WorkerLogic::hm_expire_generations_sync(const QString& timestamp)
{
    return GenerationManipulation::hm_expire_generations_wrapper(timestamp);
}

QString WorkerLogic::hm_list_generations_sync()
{
    return GenerationManipulation::hm_list_generations_wrapper();
}

// setup-nix bash scripts function handels for QT GUI.
QString WorkerLogic::install_nix_home_manager_sync(const QString& version)
{
    QStringList output;
    QStringList full_error;
    bool success = true;

    std::tie(success, output, full_error) = SetupNixHomeManager::install_nix_home_manager(version);

    QJsonObject resultObj;
    resultObj["success"] = success;
    QJsonArray arr;
    for (const QString &s : output) arr.append(s);
    resultObj["output"] = arr;
    QJsonArray arr2;
    for (const QString &s : full_error) arr2.append(s);
    resultObj["full_error"] = arr2;

    // Convert the QJsonObject to a compact JSON string and return
    return QJsonDocument(resultObj).toJson(QJsonDocument::Compact);
}

QString WorkerLogic::uninstall_nix_home_manager_sync()
{
    QStringList output;
    QStringList full_error;
    bool success = true;

    std::tie(success, output, full_error) = SetupNixHomeManager::uninstall_nix_home_manager();

    QJsonObject resultObj;
    resultObj["success"] = success;
    QJsonArray arr;
    for (const QString &s : output) arr.append(s);
    resultObj["output"] = arr;
    QJsonArray arr2;
    for (const QString &s : full_error) arr2.append(s);
    resultObj["full_error"] = arr2;

    // Convert the QJsonObject to a compact JSON string and return
    return QJsonDocument(resultObj).toJson(QJsonDocument::Compact);
}

QString WorkerLogic::detect_nix_home_manager_sync()
{
    QStringList output;
    QStringList full_error;
    bool success = true;

    std::tie(success, output, full_error) = DetectNixHomeManager::detect_nix_home_manager();

    QJsonObject resultObj;
    resultObj["success"] = success;
    QJsonArray arr;
    for (const QString &s : output) arr.append(s);
    resultObj["output"] = arr;
    QJsonArray arr2;
    for (const QString &s : full_error) arr2.append(s);
    resultObj["full_error"] = arr2;

    // Convert the QJsonObject to a compact JSON string and return
    return QJsonDocument(resultObj).toJson(QJsonDocument::Compact);
}