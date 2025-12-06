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

#include "nix-wrapper.h"

// internal functions:
QJsonArray stringListToJsonArray(const QStringList &list) {
    QJsonArray arr;
    for (const QString &s : list) arr.append(s);
    return arr;
}

QByteArray createJsonResponse(bool success, const QString& message, const QStringList& output, const QStringList& simple_error, const QStringList& full_error) {
    QJsonObject resultObj;
    resultObj["success"] = success;
    resultObj["message"] = message;
    resultObj["output"] = stringListToJsonArray(output);
    resultObj["simple_error"] = stringListToJsonArray(simple_error);
    resultObj["full_error"] = stringListToJsonArray(full_error);
    // Convert the QJsonObject to a compact JSON string and return
    return QJsonDocument(resultObj).toJson();
}

QString create_func_json_response(const QString& func_name, std::tuple<bool, QStringList, QStringList> func_tuple)
{
    qDebug() << func_name + " function invoked from QML!"; // Changed from read_packages()

    // get values out of tuple
    auto [success, output_list, full_error_list] = func_tuple;

    if (!success) {
        return createJsonResponse(
            false,
            QStringLiteral("Operation failed: Could not run function (%1). See full_error for details.").arg(func_name),
            output_list.isEmpty() ? QStringList() : output_list,
            QStringList({QStringLiteral("Encountered an error while running (%1).").arg(func_name)}),
            full_error_list.isEmpty() ? QStringList() : full_error_list
        );
    } else {
        return createJsonResponse(
            true,
            QStringLiteral("Operation Successfull: Successfull ran function (%1).").arg(func_name),
            output_list.isEmpty() ? QStringList() : output_list,
            QStringList(),
            full_error_list.isEmpty() ? QStringList() : full_error_list
        );
    }
}

namespace PackageManipulation {
    // universal output of all functions 
    // // On Success
    // {
    //   "success": true, // bool
    //   "message": "Successfully did something.", // QString
    //   "output": [ "package1", "package2" ], // QJsonArray
    //   "simple_error": QJsonArray(), // QJsonArray
    //   "full_error": QJsonArray() // QJsonArray
    // }

    // // On Failure (due to hm_switch errors)
    // {
    //   "success": false, // bool
    //   "message": "Failed to apply changes, backup restored.", // QString
    //   "output": QJsonArray(), // QJsonArray
    //   "simple_error": [ "simple_error_msg1" ], // QJsonArray
    //   "full_error": [ "full_error_line1", "full_error_line2" ] //QJsonArray
    // }

    // // On Failure (due to internal errors like JSON parsing or backup/restore)
    // {
    //   "success": false, // bool
    //   "message": "A critical error occurred.", // QString
    //   "output": QJsonArray(), // QJsonArray
    //   "simple_error": [ "User-friendly message about the internal error" ], // QJsonArray
    //   "full_error": [ "Detailed technical error message, e.g., exception details" ] // QJsonArray
    // }

    QString hm_switch_wrapper(const bool allow_insecure)
    {
        qDebug() << "hm_switch_wrapper() function invoked from QML!"; // Changed from read_packages()

        // Call the C++ backend function to read packages
        // Use the determined config path here as well, as get_config_path() is the source of truth
        QString actual_config_file_path = get_config_path();
        if (actual_config_file_path.isEmpty()) {
            return createJsonResponse(
                false,
                "Operation failed: Could not determine configuration file path.",
                QStringList(),
                QStringList({"Failed to find config file."}), // This line is fine if simple_error is meant to be a single string array
                QStringList({
                    QStringLiteral("The configuration file path could not be determined (e.g., 'echo $HOME' failed or path not found). at %1").arg(actual_config_file_path)
                })
            );
        }

        // Correctly call the backend C++ function `hm_switch`
        auto [success, output_vec, simple_error_vec, full_error_vec] = HomeManager::hm_switch(allow_insecure); 

        if (!success) {
            return createJsonResponse(
                false,
                "Operation failed: Could not update packages. See full_error for details.",
                output_vec.empty() ? QStringList() : output_vec,
                simple_error_vec.empty() ? QStringList({"Encountered an error while updating."}) : simple_error_vec,
                full_error_vec.empty() ? QStringList() : full_error_vec
            );
        } else {
            return createJsonResponse(
                true,
                "Operation Successfull: updated packages.",
                output_vec.empty() ? QStringList() : output_vec,
                simple_error_vec.empty() ? QStringList() : simple_error_vec,
                full_error_vec.empty() ? QStringList() : full_error_vec
            );
        }
    }

    QString hm_version_wrapper()
    {
        // Correctly call the backend C++ function `hm_version`
        return create_func_json_response("HomeManager::hm_version()", HomeManager::hm_version()); 
    }

    QString read_packages_wrapper(const QString& packageType)
    {
        qDebug() << "read_packages_wrapper() function invoked from QML!";

        // Call the C++ backend function to read packages
        // Use the determined config path here as well, as get_config_path() is the source of truth
        QString actual_config_file_path = get_config_path();
        if (actual_config_file_path.isEmpty()) {
            return createJsonResponse(
                false,
                "Operation failed: Could not determine configuration file path.",
                QStringList(),
                QStringList({"Failed to find config file."}),
                QStringList({
                    QStringLiteral("The configuration file path could not be determined (e.g., 'echo $HOME' failed or path not found). at %1").arg(actual_config_file_path)
                })
            );
        }

        QStringList packages = PackageOperations::read_packages(actual_config_file_path, packageType);

        // Convert the QJsonArray to a compact JSON string and return
        return createJsonResponse(
            true,
            "Operation Successfull: Read packages.",
            packages,
            QStringList(),
            QStringList()
        );
    }

    QString add_packages_wrapper(const QString& packagesJsonString, bool allow_insecure, const QString& packageType, bool overwrite)
    {
        qDebug() << "add_packages_wrapper() function invoked from QML! Packages JSON:" << packagesJsonString
                << ", Package Type:" << packageType << ", Overwrite:" << overwrite;

        // Python equivalent: if not filename: return packages_added , [[] ,["failed to find config file"], []]
        QString actual_config_file_path = get_config_path(); // Get the dynamically determined path
        if (actual_config_file_path.isEmpty()) {
            return createJsonResponse(
                false,
                "Operation failed: Could not determine configuration file path.",
                QStringList(),
                QStringList({"Failed to find config file."}),
                QStringList({
                    QStringLiteral("The configuration file path could not be determined (e.g., 'echo $HOME' failed or path not found). at %1").arg(actual_config_file_path)
                })
            );
        }

        QStringList packages_to_add;
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(packagesJsonString.toUtf8(), &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "Failed to parse packages JSON:" << parseError.errorString();
            return createJsonResponse(
                false,
                "Invalid input: Failed to parse packages JSON.",
                QStringList(),
                QStringList({"Invalid package list format."}),
                QStringList({parseError.errorString()})
            );
        }

        if (doc.isArray()) {
            QJsonArray jsonArray = doc.array();
            for (const QJsonValue& value : jsonArray) {
                if (value.isString()) {
                    packages_to_add.append(value.toString());
                } else {
                    qWarning() << "JSON array contains non-string elements. Skipping.";
                }
            }
        } else {
            qWarning() << "Expected a JSON array for 'packagesJsonString', but received a different type.";
            return createJsonResponse(
                false,
                "Invalid input: Expected a JSON array for packages.",
                QStringList(),
                QStringList({"Invalid package list format."}),
                QStringList({"Input JSON is not an array."})
            );
        }

        // --- TRANSACTIONAL LOGIC START ---

        // 1. Make a backup of the config file
        auto [backup_success, backup_msg] = backup_config_file(actual_config_file_path);
        if (!backup_success) {
            qWarning() << "Failed to create backup before adding packages:" << backup_msg;
            // Python equivalent: return packages_added , [[] ,["failed to backup too risky to run without, exiting."], [backup_error]]
            return createJsonResponse(
                false,
                "Operation failed: Could not create config backup, too risky to proceed.",
                QStringList(),
                QStringList({"Failed to create backup, operation aborted."}),
                QStringList({backup_msg})
            );
        } else {
            qDebug() << "Config backup created successfully:" << backup_msg;
        }

        // 2. Run the package addition code
        QStringList added_packages = PackageOperations::add_packages(
            actual_config_file_path, // Use the determined path
            packages_to_add,
            packageType,
            overwrite
        );

        // 3. Try to apply the config
        qDebug() << "Attempting to apply new config...";
        auto [success, output, simple_error_vec, full_error_vec] = HomeManager::hm_switch(allow_insecure); // Renamed to avoid shadowing QJsonArray simple_error/full_error

        if (!success) { // Check if hm_switch reported any errors
            qWarning() << "Failed to apply config after adding packages. Restoring backup.";

            // 4. On fail, run restore
            auto [restore_success, restore_msg] = restore_config_file(actual_config_file_path);
            // bool restore_success = true;
            // std::string restore_msg = "safety off for testing resotre did not happen!!!!!";
            if (!restore_success) {
                qCritical() << "CRITICAL ERROR: Failed to restore backup after failed hm_switch:" << restore_msg;
                // Python equivalent: simple_error.insert(0, "failed to restore you're {filename}.backup probably does not exist!")
                // full_error.insert(-1, restore_error)
                simple_error_vec.insert(0, QString("Failed to restore configuration. Your backup '%1.backup' might not exist or is corrupted.").arg(actual_config_file_path));
                full_error_vec.append(QString("Backup restore failed: %1").arg(restore_msg));

                return createJsonResponse(
                    false,
                    "CRITICAL ERROR: Failed to apply changes AND could not restore backup. Configuration might be unstable.",
                    QStringList(),
                    simple_error_vec,
                    full_error_vec

                );
            } else {
                qDebug() << "Backup restored successfully:" << restore_msg;
                // Python equivalent: return packages_added, [output , simple_error, full_error]
                return createJsonResponse(
                    false,
                    "Failed to apply changes, backup restored. Please check your configuration.",
                    QStringList(),
                    simple_error_vec,
                    full_error_vec
                );
            }

        } else {
            qDebug() << "Config applied successfully after adding packages.";
            // Python equivalent: return packages_added, [output, simple_error, full_error]
            return createJsonResponse(
                true,
                "Operation Successfull: Added packages.",
                added_packages,
                QStringList(),
                QStringList()
            );
        }
        // --- TRANSACTIONAL LOGIC END ---
    }

    QString delete_packages_wrapper(const QString& packagesJsonString, const QString& packageType)
    {
        qDebug() << "delete_packages_wrapper() function invoked from QML! Packages JSON:" << packagesJsonString
                << ", Package Type:" << packageType;

        // Python equivalent: if not filename: return packages_deleted , [[] ,["failed to find config file"], []]
        QString actual_config_file_path = get_config_path(); // Get the dynamically determined path
        if (actual_config_file_path.isEmpty()) {
            return createJsonResponse(
                false,
                "Operation failed: Could not determine configuration file path.",
                QStringList(),
                QStringList({"Failed to find config file."}),
                QStringList({
                    QStringLiteral("The configuration file path could not be determined (e.g., 'echo $HOME' failed or path not found). at %1").arg(actual_config_file_path)
                })
            );
        }

        QStringList packages_to_delete;
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(packagesJsonString.toUtf8(), &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "Failed to parse packages JSON:" << parseError.errorString();
            return createJsonResponse(
                false,
                "Invalid input: Failed to parse packages JSON.",
                QStringList(),
                QStringList({"Invalid package list format."}),
                QStringList({parseError.errorString()})
            );
        }

        if (doc.isArray()) {
            QJsonArray jsonArray = doc.array();
            for (const QJsonValue& value : jsonArray) {
                if (value.isString()) {
                    packages_to_delete.append(value.toString());
                } else {
                    qWarning() << "JSON array contains non-string elements. Skipping.";
                }
            }
        } else {
            qWarning() << "Expected a JSON array for 'packagesJsonString', but received a different type.";
            return createJsonResponse(
                false,
                "Invalid input: Expected a JSON array for packages.",
                QStringList(),
                QStringList({"Invalid package list format."}),
                QStringList({"Input JSON is not an array."})
            );
        }

        // --- TRANSACTIONAL LOGIC START ---

        // 1. Make a backup of the config file
        auto [backup_success, backup_msg] = backup_config_file(actual_config_file_path);
        if (!backup_success) {
            qWarning() << "Failed to create backup before deleting packages:" << backup_msg;
            // Python equivalent: return packages_deleted , [[] ,["failed to backup too risky to run without, exiting."], [backup_error]]
            return createJsonResponse(
                false,
                "Operation failed: Could not create config backup, too risky to proceed.",
                QStringList(),
                QStringList({"Failed to create backup, operation aborted."}),
                QStringList({backup_msg})
            );
        } else {
            qDebug() << "Config backup created successfully:" << backup_msg;
        }

        // 2. Run the package deletion code
        QStringList deleted_packages = PackageOperations::delete_packages(actual_config_file_path, packages_to_delete, packageType.isEmpty() ? QString() : packageType);

        // 3. Try to apply the config
        qDebug() << "Attempting to apply new config...";
        auto [success, output, simple_error_vec, full_error_vec] = HomeManager::hm_switch(true); // Renamed to avoid shadowing (true on allow insecure because we are deleting no point in specifying it in delete function)

        if (!success) { // Check if hm_switch reported any errors
            qWarning() << "Failed to apply config after deleting packages. Restoring backup.";

            // 4. On fail, run restore
            auto [restore_success, restore_msg] = restore_config_file(actual_config_file_path);
            if (!restore_success) {
                qCritical() << "CRITICAL ERROR: Failed to restore backup after failed hm_switch:" << restore_msg;
                // Python equivalent: simple_error.insert(0, "failed to restore you're {filename}.backup probably does not exist!")
                // full_error.insert(-1, restore_error)
                simple_error_vec.insert(0, QString("Failed to restore configuration. Your backup '%1.backup' might not exist or is corrupted.").arg(actual_config_file_path));
                full_error_vec.append(QString("Backup restore failed: %1").arg(restore_msg));

                return createJsonResponse(
                    false,
                    "CRITICAL ERROR: Failed to apply changes AND could not restore backup. Configuration might be unstable.",
                    QStringList(),
                    simple_error_vec,
                    full_error_vec
                );

            } else {
                qDebug() << "Backup restored successfully:" << restore_msg;
                // Python equivalent: return packages_deleted, [output , simple_error, full_error]
                return createJsonResponse(
                    false,
                    "Failed to apply changes, backup restored. Please check your configuration.",
                    QStringList(),
                    simple_error_vec,
                    full_error_vec
                );
            }

        } else {
            qDebug() << "Config applied successfully after deleting packages.";
            // Python equivalent: return packages_deleted, [output, simple_error, full_error]
            return createJsonResponse(
                true,
                "Operation Successfull: Read packages.",
                deleted_packages,
                QStringList(),
                QStringList()
            );
        }
        // --- TRANSACTIONAL LOGIC END ---
    }

    QString search_packages_wrapper(const QString& quarry, const bool local, const QString& base_url, const int timeout)
    {
        qDebug() << "search_packages_wrapper() function invoked from QML!, redirecting to quarry functions.";
        
        if (local) {
            return create_func_json_response("NixEnv::quarry(quarry)", NixEnv::quarry(quarry)); 
        } else {
            return create_func_json_response("NixHubAPI::quarry(quarry , base_url)", NixHubAPI::quarry(quarry , base_url, timeout)); 
        }
    }
}

namespace ChannelManipulation {
    // universal output of all functions 
    // // On Success
    // {
    //   "success": true, // bool
    //   "message": "Successfully did something.", // QString
    //   "output": [ "package1", "package2" ], // QJsonArray
    //   "simple_error": QJsonArray(), // QJsonArray
    //   "full_error": QJsonArray() // QJsonArray
    // }

    // // On Failure (due to hm_switch errors)
    // {
    //   "success": false, // bool
    //   "message": "Failed to apply changes, backup restored.", // QString
    //   "output": QJsonArray(), // QJsonArray
    //   "simple_error": [ "simple_error_msg1" ], // QJsonArray
    //   "full_error": [ "full_error_line1", "full_error_line2" ] //QJsonArray
    // }

    // // On Failure (due to internal errors like JSON parsing or backup/restore)
    // {
    //   "success": false, // bool
    //   "message": "A critical error occurred.", // QString
    //   "output": QJsonArray(), // QJsonArray
    //   "simple_error": [ "User-friendly message about the internal error" ], // QJsonArray
    //   "full_error": [ "Detailed technical error message, e.g., exception details" ] // QJsonArray
    // }

    QString update_channels_wrapper()
    {
        // Correctly call the backend C++ function `update_channel`
        return create_func_json_response("NixChannel::update_channels()", NixChannel::update_channels()); 
    }

    QString list_channels_wrapper()
    {
        // Correctly call the backend C++ function `list_channels`
        return create_func_json_response("NixChannel::list_channels()", NixChannel::list_channels()); 
    }

    QString add_channels_wrapper(const QString& url, const QString& name)
    {
        // Correctly call the backend C++ function `add_channel`
        return create_func_json_response("NixChannel::add_channel(url, name)", NixChannel::add_channel(url, name)); 
    }

    QString remove_channels_wrapper(const QString& name)
    {
        // The reason this I commented out is because I decided there is a limit to how much I want to hold the users hand, and because some users may want to switch channels releases in the feature.
        // QJsonObject resultObj;

        // if (name.toStdString() == "home-manager") {
        //     resultObj["success"] = false;
        //     resultObj["message"] = "Operation failed: Could not remove channel. See full_error for details.";
        //     resultObj["output"] = QJsonArray();
        //     resultObj["simple_error"] = QJsonArray({"Encountered an error while removing channel."}); // This is fine if a generic message is desired
        //     resultObj["full_error"] = QJsonArray({"Channel is protected removing it will corrupt the Nix Installation!!!"});
        //     return QJsonDocument(resultObj).toJson(QJsonDocument::Compact);
        // } else if (name.toStdString() == "nixpkgs") {
        //     resultObj["success"] = false;
        //     resultObj["message"] = "Operation failed: Could not remove channel. See full_error for details.";
        //     resultObj["output"] = QJsonArray();
        //     resultObj["simple_error"] = QJsonArray({"Encountered an error while removing channel."}); // This is fine if a generic message is desired
        //     resultObj["full_error"] = QJsonArray({"Channel is protected removing it will corrupt the Nix Installation!!!"});
        //     return QJsonDocument(resultObj).toJson(QJsonDocument::Compact);
        // }

        // Correctly call the backend C++ function `remove_channel`
        return create_func_json_response("NixChannel::remove_channel(name)", NixChannel::remove_channel(name)); 
    }
}

namespace GenerationManipulation {
    // universal output of all functions 
    // // On Success
    // {
    //   "success": true, // bool
    //   "message": "Successfully did something.", // QString
    //   "output": [ "package1", "package2" ], // QJsonArray
    //   "simple_error": QJsonArray(), // QJsonArray
    //   "full_error": QJsonArray() // QJsonArray
    // }

    // // On Failure (due to hm_switch errors)
    // {
    //   "success": false, // bool
    //   "message": "Failed to apply changes, backup restored.", // QString
    //   "output": QJsonArray(), // QJsonArray
    //   "simple_error": [ "simple_error_msg1" ], // QJsonArray
    //   "full_error": [ "full_error_line1", "full_error_line2" ] //QJsonArray
    // }

    // // On Failure (due to internal errors like JSON parsing or backup/restore)
    // {
    //   "success": false, // bool
    //   "message": "A critical error occurred.", // QString
    //   "output": QJsonArray(), // QJsonArray
    //   "simple_error": [ "User-friendly message about the internal error" ], // QJsonArray
    //   "full_error": [ "Detailed technical error message, e.g., exception details" ] // QJsonArray
    // }


    QString list_generations_wrapper()
    {
        // Correctly call the backend C++ function `list_generations`
        return create_func_json_response("NixEnv::list_generations()", NixEnv::list_generations());
    }

    QString switch_generation_wrapper(const QString& generation_id)
    {
        // Correctly call the backend C++ function `switch_generation`
        return create_func_json_response("NixEnv::switch_generation(generation_id)", NixEnv::switch_generation(generation_id));
    }

    QString delete_generation_wrapper(const QString& generation_id)
    {
        // Correctly call the backend C++ function `delete_generation`
        return create_func_json_response("NixEnv::delete_generation(generation_id)", NixEnv::delete_generation(generation_id));  
    }

    QString delete_old_generations_wrapper()
    {
        // Correctly call the backend C++ function `delete_old_generations`
        return create_func_json_response("NixEnv::delete_old_generations()", NixEnv::delete_old_generations());
    }

    QString hm_expire_generations_wrapper(const QString& timestamp)
    {
        // Correctly call the backend C++ function `hm_expire_generations`
        return create_func_json_response("HomeManager::hm_expire_generations(timestamp)", HomeManager::hm_expire_generations(timestamp));
    }

    QString hm_list_generations_wrapper()
    {
        // Correctly call the backend C++ function `hm_list_generations`
        return create_func_json_response("HomeManager::hm_list_generations()", HomeManager::hm_list_generations());
    }
}