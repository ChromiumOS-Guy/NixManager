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

// Include nlohmann/json header

#include "nixhub-api.h" // header


std::tuple<bool, QString> httpGetRawUrl(const QString &rawUrl, int timeoutMs) {
    QUrl url = QUrl::fromEncoded(rawUrl.toUtf8()); // don't forget encoding!!
    QNetworkRequest req(url); // create rq object
    req.setHeader(QNetworkRequest::UserAgentHeader, "nixhub/1.0");
    static QNetworkAccessManager mgr;           // keep alive across calls, avoids destroying it while replies are still running and reuses sockets/IO efficiency
    QEventLoop loop; 
    QNetworkReply *reply = mgr.get(req); 
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit); // Connects the reply's finished signal to the event loop's quit slot
    QTimer timer; timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit); // Connects the timer's timeout signal to the same event loop quit
    timer.start(timeoutMs);
    loop.exec();
    if (!timer.isActive()) { reply->abort(); reply->deleteLater(); return {false, QStringLiteral("timeout")}; } // If the timer is no longer active it means it fired (timeout occurred) before we explicitly stopped it. 
    if (reply->error() != QNetworkReply::NoError) {
        QString err = reply->errorString();
        reply->deleteLater();
        return {false, err};
    }
    QByteArray data = reply->readAll();
    reply->deleteLater();
    return {true, QString::fromUtf8(data)};
}

namespace NixHubAPI {
    // Function to fetch package details
    std::tuple<bool, QJsonObject, QString>
    fetchPackageDetails(const QString& package_id, const QString& base_url, int timeoutMs) {
        const QString api_heading = "/v2/pkg?name=";
        QString assembled_url = base_url + api_heading + package_id; // for example: https://search.devbox.sh/v2/pkg?name=firefox (base_url being: https://search.devbox.sh , package_id being: firefox)

        QString errors = ""; //init QString
        QString output;
        bool success;
        std::tie(success, output) = httpGetRawUrl(assembled_url, timeoutMs);
        if (!success) { // if not successful check why
            if (output == "timeout") { // if timeout log as such
                errors += "\nRequest timeout";
            } else { // else put somethign generic
                errors += "\nRequest failed";
            }
            return {false, QJsonObject(), errors};
        } else if (output.isEmpty()) { // if empty log as such
            errors += "\nResponse Empty";
            return {false, QJsonObject(), errors};
        }

        const QByteArray bytes = output.toUtf8(); // encode utf8
        QJsonParseError parseError;
        const QJsonDocument doc = QJsonDocument::fromJson(bytes, &parseError); // parse QString to QJsonDocument or log errors and return
        if (parseError.error != QJsonParseError::NoError || doc.isNull()) {
            errors += "\nJSON parse error: " + parseError.errorString();
            return {false, QJsonObject(), errors};
        }

        if (!doc.isObject()) { // check QJsonDocument is valid
            errors += "\nJSON root is not an object";
            return {false, QJsonObject(), errors};
        }

        return {true, doc.object(), errors}; // return API results as QJsonObject
    }

    std::tuple<bool, QStringList, QStringList>
    quarry(const QString& quarry, const QString& base_url, const int timeoutS)
    {
        QString api_heading = "/v2/search?q=";
        QString assembled_url = base_url + api_heading + quarry; // for example: https://search.devbox.sh/v2/search?q=firefox (base_url being: https://search.devbox.sh , quarry being: firefox)

        QString output;
        QString full_error;
        bool success;

        std::tie(success, output) = httpGetRawUrl(assembled_url, timeoutS*1000);
        if (!success) { // if not successful check why
            if (output == "timeout") { // if timeout log as such
                full_error += "\nRequest timeout";
            } else { // else put somethign generic
                full_error += "\nRequest failed";
            }
            return {false, {output}, {full_error}};
        } else if (output.isEmpty()) { // if empty log as such
            full_error += "\nResponse Empty";
            return {false, {output}, {full_error}};
        }

        if (!success || output.isEmpty()) {
            full_error.push_back("Either failed or output was empty");
            return {false, {output}, {full_error}};
        }
        
        // 3. Parse the full JSON string
        QJsonParseError parseError;
        QJsonDocument output_doc = QJsonDocument::fromJson(output.toUtf8(), &parseError);

        if (parseError.error != QJsonParseError::NoError || !output_doc.isObject()) {
            // Restore original output and report error. (refuse to serve potentially incompatible packages)
            full_error += "\nFailed to parse JSON response or response was not a JSON object.";
            return {false, {output}, {full_error}};
        }

        QJsonObject root = output_doc.object();

        // 4. Extract the "results" array as a QJsonValue
        QJsonValue resultsValue = root.value("results"); // IMPORTANT {"query":"quarry","total_results":50,"results": [ACTUAL DATA WE WANT]}

        if (!resultsValue.isArray()) {
            // Restore original output and report error
            full_error += "\nJSON object does not contain a valid 'results' array.";
            return {false, {output}, {full_error}};
        }

        QJsonArray output_packages_array = resultsValue.toArray(); // translate from QJsonValue to QJsonArray

        // ---------------------------------------------------------------------
        // PACKAGE FILTERING LOGIC START
        // ---------------------------------------------------------------------
        
        // The accumulator MUST be a QJsonArray to use Qt JSON methods like append/isEmpty.
        QStringList filtered_packages_qjson; 

        // Iterate over the initial list of packages
        for (const auto& package_value : output_packages_array) {
            
            // Ensure the iterated item is a QJsonObject before calling toObject()
            if (!package_value.isObject()) continue; 
            QJsonObject package_object = package_value.toObject();

            // Check if the package has a name
            if (!package_object.contains("name") || !package_object["name"].isString()) continue;
            
            // Use the QJsonObject to get the package_id
            QString package_id = package_object["name"].toString();
            
            QJsonObject package_details;
            QString fetch_error;
            
            // Fetch detailed package info (releases, platforms, etc.)
            std::tie(success, package_details, fetch_error) = fetchPackageDetails(package_id, base_url, timeoutS*1000);

            if (!success) {
                full_error += "\ncould not find package - " + package_id; // get package_id for logs
                full_error += "\n" + fetch_error; // get actual error for logs
                continue; // Move to the next package
            }

            bool arch_match_found = false;
            if (package_details.contains("releases") && package_details["releases"].isArray()) { // {"name":"","summary":"","homepage_url":"","license":"","releases":[WHAT WE WANT]}
                QJsonArray releasesArray = package_details["releases"].toArray();
                for (const auto& release : releasesArray) {
                    if (!release.isObject()) continue;
                    QJsonObject releaseObject = release.toObject(); 

                    if (releaseObject.contains("platforms") && releaseObject["platforms"].isArray()) { // "releases":[{"version":"","last_updated":"","platforms":[WHAT WE WANT]}]
                        QJsonArray platformsArray = releaseObject["platforms"].toArray();
                        for (const auto& platform : platformsArray) {
                            if (!platform.isObject()) continue;
                            QJsonObject platformObject = platform.toObject();

                            // Check for architecture match
                            if (platformObject.contains("arch") && platformObject["arch"].isString() && // "platforms":[{"arch": "WHAT WE WANT"},..,..,..]
                                platformObject["arch"].toString().toStdString() == (ARCH == "aarch64" ? "arm64" : "x86-64")) // if its not arm64 its x86-64, ARCH macro will return x86_64 we need x86-64 hence this ternary operator.
                            { 
                                // FIX: use package_object to ensure there is actually a valid JSON appended unlike using package_value
                                filtered_packages_qjson.append(QJsonDocument(package_object).toJson());
                                arch_match_found = true;
                                break; // Break out of the platform loop
                            }
                        }
                    }
                    if (arch_match_found) break; // Break out of the release loop
                }
            }
        }

        // Check Use QJsonArray::isEmpty() on filtered_packages_qjson
        if (filtered_packages_qjson.isEmpty()) {
            full_error += "\nNo packages found with matching architecture! (" + QString::fromStdString((ARCH == "aarch64" ? "arm64" : "x86-64")) + ")";
            // Return failure status and empty vector of results
            return {false, {output}, {full_error}};
        }

        // Return success status (based on filtering result), the converted string vector, and all errors
        return {true, filtered_packages_qjson, {full_error}};
    }
}