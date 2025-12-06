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


#ifndef NIX_WRAPPER_H
#define NIX_WRAPPER_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QStringList>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

// custom libs
#include "nix-config.h" // read/add/delete
#include "backup-config.h" // backup/restore/config_path
#include "nix-interact.h" // apply/update/detect
#include "nixhub-api.h" // search

namespace PackageManipulation {

    /**
    * @brief Applies the Nix configuration by invoking the backend logic.
    *
    * This function retrieves the configuration file path and checks its validity.
    * If the path is valid, it calls the backend function to apply the configuration,
    * capturing both standard output and error details. The results are structured
    * into a JSON object, indicating success or failure, along with any relevant
    * error messages. If the configuration file path cannot be determined, an
    * appropriate error message is returned.
    *
    * @param allow_insecure A boolean flag indicating whether to allow insecure 
    * packages to be operated on during the configuration application.
    *
    * @return A JSON string representing the result of the configuration application,
    * including success status, messages, and any error details.
    */
    // QString hm_switch_wrapper(const bool allow_insecure = false);
    QString hm_switch_wrapper(const bool allow_insecure);

    /**
    * @brief Detects version of home-manager.
    *
    * This function attempts to get the version of the currently installed home-manager
    *
    * @return A JSON string representing the result of the get version operation.
    * On success, it returns a JSON object with a success message and a QJsonArray
    * of channel names. On failure, it returns a JSON object with error details.
    */
    QString hm_version_wrapper();

    /**
    * @brief Reads the list of packages of a specific type from the Nix configuration file.
    *
    * This function interacts with the backend logic to parse the user's home.nix
    * or similar configuration file and extract packages defined within a specified
    * package block (e.g., `home.packages`).
    *
    * @param packageType The type of package block to read (e.g., "home", "system").
    * Defaults to "home".
    * @return A JSON string representing a QJsonArray of package names on success,
    * or a JSON object with error details on failure.
    */
    // QString read_packages_wrapper(const QString& packageType = QString::fromStdString("home"));
    QString read_packages_wrapper(const QString& packageType);

    /**
    * @brief Adds new packages to the Nix configuration file.
    *
    * This function takes a JSON string of packages and adds them to the specified
    * package block in the user's Nix configuration. It includes transactional logic
    * with backup and restore, and attempts to apply the configuration after modification.
    *
    * @param packagesJsonString A JSON string representing a QJsonArray of package names to add.
    * @param packageType The type of package block to modify (e.g., "home", "system").
    * Defaults to "home".
    * @param overwrite If true, existing packages in the block are replaced by the new list.
    * If false, new packages are appended. Defaults to false.
    * @param allow_insecure allows the installation of insecure packages with known CVE.
    * @return A JSON string indicating success/failure, along with data (added packages)
    * or error messages and output from the Nix build process.
    */
    // QString add_packages_wrapper(const QString& packagesJsonString, bool allow_insecure = false, const QString& packageType = QString::fromStdString("home"), bool overwrite = false);
    QString add_packages_wrapper(const QString& packagesJsonString, bool allow_insecure, const QString& packageType, bool overwrite);

    /**
     * @brief Deletes specified packages from the Nix configuration file.
     *
     * This function takes a JSON string of packages and removes them from the specified
     * package block in the user's Nix configuration. It includes transactional logic
     * with backup and restore, and attempts to apply the configuration after modification.
     *
     * @param packagesJsonString A JSON string representing a QJsonArray of package names to delete.
     * @param packageType The type of package block to modify (e.g., "home", "system").
     * Defaults to "home".
     * @return A JSON string indicating success/failure, along with data (deleted packages)
     * or error messages and output from the Nix build process.
     */
    // QString delete_packages_wrapper(const QString& packagesJsonString, const QString& packageType = QString::fromStdString("home"));
    QString delete_packages_wrapper(const QString& packagesJsonString, const QString& packageType);

    /**
    * @brief Searches for packages based on a query string.
    *
    * This function queries the Nix package manager or a remote API to find packages
    * that match the specified search term. It handles both local and remote searches,
    * returning results in a structured JSON format. If the search is successful, it
    * filters the results based on the system architecture.
    *
    * @param quarry A string representing the search term for the package query.
    * @param local A boolean indicating whether to perform a local search (true) 
    * or a remote search (false).
    * @param base_url A string representing the URL for the remote search API. Defaults 
    * to "https://search.devbox.sh".
    * @return A JSON string indicating success or failure, along with the search 
    * results or error messages.
    */
    // QString search_packages_wrapper(const QString& quarry, const bool local = false, const QString& base_url = QString::fromStdString("https://search.devbox.sh"));
    QString search_packages_wrapper(const QString& quarry, const bool local, const QString& base_url, const int timeout);
}

namespace ChannelManipulation {

     /**
    * @brief Updates the Nix package manager channels.
    *
    * This function invokes the backend logic to update channels
    *
    * @return A JSON string representing the result of the update operation.
    * On success, it returns a JSON object with a success message and a QJsonArray
    * of updated package names. On failure, it returns a JSON object with error details.
    *
    * Example success: `{"success": true, "message": "Operation Successful: updated channels.", "output": ["lineA", "lineB"], "simple_error": [], "full_error": []}`
    * Example failure: `{"success": false, "message": "Operation failed: Could not update channels. See full_error for details.", "simple_error": ["Encountered an error while updating channels."], "full_error": ["Error details..."]}`
    */
    QString update_channels_wrapper();

    /**
    * @brief Lists the Nix package manager channels.
    *
    * This function invokes the backend logic to retrieve the currently configured Nix channels.
    *
    * @return A JSON string representing the result of the list operation.
    * On success, it returns a JSON object with a success message and a QJsonArray
    * of channel names. On failure, it returns a JSON object with error details.
    */
    QString list_channels_wrapper();

    /**
    * @brief Adds a new Nix channel.
    *
    * This function invokes the backend logic to add a new Nix channel
    * specified by the provided URL and name.
    *
    * @param url The URL of the channel to be added.
    * @param name The name to assign to the new channel.
    *
    * @return A JSON string representing the result of the add operation.
    * On success, it returns a JSON object with a success message and a
    * QJsonArray of the added channel details. On failure, it returns a
    * JSON object with error details.
    *
    */
    QString add_channels_wrapper(const QString& url, const QString& name);

    /**
    * @brief Removes an existing Nix channel.
    *
    * This function invokes the backend logic to remove a Nix channel
    * specified by the provided name.
    *
    * @param name The name of the channel to be removed.
    *
    * @return A JSON string representing the result of the remove operation.
    * On success, it returns a JSON object with a success message and a
    * QJsonArray of the removed channel details. On failure, it returns a
    * JSON object with error details.
    */
    QString remove_channels_wrapper(const QString& name);
}

namespace GenerationManipulation { // no need to implement generation creation as that happens everytime the config file is applied.

    /**
    * @brief Lists the Nix package manager generations.
    *
    * This function invokes the backend logic to retrieve the currently configured Nix generations.
    *
    * @return A JSON string representing the result of the list operation.
    * On success, it returns a JSON object with a success message and a QJsonArray
    * of channel names. On failure, it returns a JSON object with error details.
    */
    QString list_generations_wrapper();

    /**
    * @brief switches to an existing Nix generation.
    *
    * This function invokes the backend logic to switch to a Nix generation
    * specified by the provided generation_id.
    *
    * @param generation_id A string representing the ID of the generation to be switched to.
    *
    * @return A JSON string representing the result of the switch operation.
    * On success, it returns a JSON object with a success message and a
    * QJsonArray of the removed channel details. On failure, it returns a
    * JSON object with error details.
    */
    QString switch_generation_wrapper(const QString& generation_id);

    /**
    * @brief Deletes an existing Nix generation.
    *
    * This function invokes the backend logic to delete a Nix generation
    * specified by the provided generation_id.
    *
    * @param generation_id The generation_id of the channel to be removed.
    *
    * @return A JSON string representing the result of the delete operation.
    * On success, it returns a JSON object with a success message and a
    * QJsonArray of the removed channel details. On failure, it returns a
    * JSON object with error details.
    */
    QString delete_generation_wrapper(const QString& generation_id);

    /**
    * @brief Deletes all old existing Nix generations.
    *
    * This function invokes the backend logic to delete old Nix generations
    *
    * @return A JSON string representing the result of the delete operation.
    * On success, it returns a JSON object with a success message and a
    * QJsonArray of the removed channel details. On failure, it returns a
    * JSON object with error details.
    */
    QString delete_old_generations_wrapper();

    /**
    * @brief Deletes all non-current generations according to timestamp.
    *
    * This function attempts to expire (delete) home-manager genrations older then date N.
    *
    * @return A JSON string representing the result of the expire operation.
    * On success, it returns a JSON object with a success message and a QJsonArray
    * of channel names. On failure, it returns a JSON object with error details.
    */
    // QString hm_expire_generations_wrapper(const std::string& timestamp = "-30 days");
    QString hm_expire_generations_wrapper(const QString& timestamp);

    /**
    * @brief Lists the home-manager generations.
    *
    * This function invokes the backend logic to list all home-manager generations.
    *
    * @return A JSON string representing the result of the list operation.
    * On success, it returns a JSON object with a success message and a QJsonArray
    * of channel names. On failure, it returns a JSON object with error details.
    */
    QString hm_list_generations_wrapper();
}

#endif // NIX_WRAPPER_H