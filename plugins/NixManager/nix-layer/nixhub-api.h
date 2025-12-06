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
 * docs for nixhub API (v2): https://www.jetify.com/docs/nixhub/
 */

#ifndef NIXHUB_API_H
#define NIXHUB_API_H

#include <tuple> // For std::
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QNetworkReply>
#include <QTimer>

namespace NixHubAPI {
    /**
    * @brief Fetch package details by name from the search API and return as a QJsonObject.
    *
    * Sends an HTTP GET request to the package endpoint and parses the response into
    * a QJsonObject. Returns a tuple: success flag, parsed QJsonObject (empty on
    * failure), and a vector of error messages collected during the operation.
    *
    * @param package_id The unique identifier (name) of the package to fetch.
    * @param base_url Base URL of the search API (e.g., "https://search.devbox.sh").
    * @param timeout Request timeout in seconds passed to the wget command.
    * @return std::tuple<bool, QJsonObject, std::vector<std::string>> where:
    *   - bool: true if the request succeeded and JSON was parsed, false otherwise.
    *   - QJsonObject: parsed package details on success, empty object on failure.
    *   - QString: list of error messages (empty on success).
    */
    std::tuple<bool, QJsonObject, QString>
    fetchPackageDetails(const QString& package_id, const QString& base_url, int timeoutMs);

    /**
    * @brief Queries for a specified package using a given URL of the NixHub API (v2), Filter packages by architecture.
    *
    * This function queries the NixHub API (v2) to find information about a package
    * Iterates over a QJsonArray of packages and returns only those compatible with the specified architecture.
    * based on its name. It constructs a request URL using the provided package name
    * and an optional custom URL. The function returns a tuple indicating success or
    * failure, an error message (if any), and the search results in JSON format.
    *
    * @param quarry The name of the package to search for.
    * @param base_url An optional URL for the search API. Defaults to 
    *            "https://search.devbox.sh", where %s is replaced 
    *            with the package name.
    * @param timeout Request timeout in seconds passed to the wget command.
    * @return A tuple containing:
    * - bool success: True if the query was successful, false otherwise.
    * - QStringList results: A JSON (like) Array containing the search results.
    * - QStringList error_message: An error message (empty if successful).
    */
    std::tuple<bool, QStringList, QStringList> quarry(const QString& quarry, const QString& base_url, const int timeoutS);
}
#endif // NIXHUB_API_H