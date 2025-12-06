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

#include "nix-interact.h" // Include the declarative header

namespace HomeManager {
    std::tuple<bool, QStringList, QStringList, QStringList>
    hm_switch(const bool allow_insecure) {
        // Define the error prefix
        const QString error_prefix = QStringLiteral("error: attribute");

        // Initialize the list for simple errors
        QStringList simple_error;
        QStringList output;
        QStringList full_error;
        bool success;

        // enable/disable insecure packages during switch
        QString command = QStringLiteral("home-manager switch");
        if (allow_insecure) { // adds env variable NIXPKGS_ALLOW_INSECURE=1 which enables insecure packages.
            command = QStringLiteral("export NIXPKGS_ALLOW_INSECURE=1 && home-manager switch");
        }

        std::tie(success, output, full_error) = exec_bash(command);


        for (const QString &line : full_error) {
            int pos = line.indexOf(error_prefix);
            if (pos != -1) {
                // Extract everything after the error_prefix
                // And trim leading/trailing whitespace from the extracted message
                QString extracted = line.mid(pos + error_prefix.length()).trimmed();
                simple_error << extracted;
            }
        }

        return std::make_tuple(success, output, simple_error, full_error);
    }


    std::tuple<bool, QStringList, QStringList>
    hm_version() {
        // Initialize the lists for output and full error
        QStringList output;
        QStringList full_error;
        bool success;

        std::tie(success, output, full_error) = exec_bash(QStringLiteral("home-manager --version"));

        // Return one list and the bool as a tuple
        return {success, output, full_error}; // for failure
    }

    std::tuple<bool, QStringList, QStringList>
    hm_expire_generations(const QString& timestamp) {
        // Initialize the lists for output and full error
        QStringList output;
        QStringList full_error;
        QString command = QStringLiteral("home-manager expire-generations '%1'").arg(timestamp);
        bool success;

        std::tie(success, output, full_error) = exec_bash(command);

        // Return one list and the bool as a tuple
        return {success, output, full_error}; // for failure
    }

    std::tuple<bool, QStringList, QStringList>
    hm_list_generations() {
        // Initialize the lists for output and full error
        QStringList full_error;
        QStringList output;
        QStringList result;
        bool success;

        std::tie(success, output, full_error) = exec_bash(QStringLiteral("home-manager generations"));

        // Return one list and the bool as a tuple
        if (output.isEmpty()) {
            return {success, {}, QStringList{QStringLiteral("No generations found!")}}; // for failure
        } else if (!full_error.isEmpty()) {
            return {success, {}, full_error}; // for failure
        }

        // example of healthy output before:
        // 2025-09-26 11:27 : id 3 -> /nix/store/...-home-manager-generation

        // Regex to capture: datetime (YYYY-...), ":", "id" <number>, "->" <path>
        // Uses a permissive datetime capture to include spaces.
        QRegularExpression re(QStringLiteral(R"(^\s*(\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2})\s*:\s*id\s+(\d+)\s*->\s*(.+)$)"),
                            QRegularExpression::MultilineOption);

        for (const QString &line : output) {
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                QString datetime = match.captured(1).trimmed();
                QString id = match.captured(2).trimmed();
                QString path = match.captured(3).trimmed();

                // Create a JSON-like string for each generation (as in the original)
                result << QStringLiteral("{\"id\": \"%1\", \"datetime\": \"%2\", \"path\": \"%3\"}")
                            .arg(id, datetime, path);
            }
        }

        // example of healthy output after:
        // [
        //     {"id" : "3", "datetime" : "2025-09-26 11:27" , "path" : "/nix/store/..."},
        //     {"id" : "1", "datetime" : "2025-09-25 16:48" , "path" : "/nix/store/..."}
        // ]

        return {success, result, full_error}; // for success

        
    }
}


namespace NixEnv {

    std::tuple<bool, QStringList, QStringList>
    quarry(const QString& quarry) {
        // Initialize the lists for output and full error
        QStringList full_error;
        QStringList output;
        QString command = QStringLiteral("nix-env -qaP %1").arg(quarry); // Replace "%s" placeholder with quarry
        QStringList result;
        bool success;

        std::tie(success, output, full_error) = exec_bash(command);

        // Return on failure conditions
        if (!success) {
            return {success, output, full_error};
        }
        if (output.isEmpty()) {
            return {false, output, QStringList{QStringLiteral("No packages found!")}};
        }
        if (output == QStringList{QStringLiteral("Killed")}) {
            return {false, output, QStringList{QStringLiteral("Command timed out!")}};
        }

        // Example of healthy output before:
        // nixpkgs.firefox-esr      firefox-140.3.0esr
        // nixpkgs.firefox          firefox-143.0
        // ...

        // Process the output to extract package names and summaries
        for (const QString &line : output) {
            // Split on whitespace into at most 2 parts (pkg_name and summary)
            QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            if (parts.size() < 2) continue;

            QString pkg_name = parts.at(0);
            QString summary = parts.mid(1).join(' '); // join remainder as summary

            // Remove leading namespace "nixpkgs." if present
            int dotPos = pkg_name.indexOf('.');
            if (dotPos != -1) pkg_name = pkg_name.mid(dotPos + 1);

            // Create JSON-like structured string (compatibility with original)
            QString structured_output = QStringLiteral("{\"name\": \"%1\", \"summary\": \"%2\", \"last_updated\": \"\"}")
                                            .arg(pkg_name, summary);
            result << structured_output;
        }

        // Example of healthy output after:
        // [
        //   {"name":"firefox-esr","summary":"firefox-140.3.0esr","last_updated":""}, ...
        // ]

        return {success, result, full_error};
        
    }

    std::tuple<bool, QStringList, QStringList>
    list_generations() {
        // Initialize the lists for output and full error
        QStringList full_error;
        QStringList output;
        QStringList result;
        bool success;

        std::tie(success, output, full_error) = exec_bash(QStringLiteral("nix-env --list-generations"));

        // Return early on failure conditions
        if (!success) {
            return {success, output, full_error}; // process failed
        }
        if (output.isEmpty()) {
            return {false, {}, QStringList{QStringLiteral("No generations found!")}}; // no output
        }

        // Example input lines:
        // 1   2025-09-25 16:47:18
        // 2   2025-09-25 16:48:56
        // 3   2025-09-26 08:12:16   (current)

        // For each line, parse: id, datetime, optional "(current)"
        for (const QString &line : output) {
            // Normalize whitespace and split into parts
            QString normalized = line.trimmed();
            if (normalized.isEmpty()) continue;

            // Split into columns: id, date, time, optional marker(s)
            QStringList parts = normalized.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            if (parts.size() < 3) continue; // need at least id, date, time

            QString id = parts.at(0);
            QString datetime = parts.at(1) + QStringLiteral(" ") + parts.at(2);

            // Determine if "(current)" appears anywhere in the original line
            bool is_current = normalized.contains(QStringLiteral("(current)"));

            // Build JSON-like structured string matching original style
            result << QStringLiteral("{\"id\": \"%1\", \"datetime\": \"%2\", \"is_current\": %3}")
                        .arg(id, datetime, is_current ? QStringLiteral("true") : QStringLiteral("false"));
        }

        // Example transformed output:
        // {"id":"1","datetime":"2025-09-25 16:47:18","is_current":false}, ...

        return {success, result, full_error};

        
    }

    std::tuple<bool, QStringList, QStringList>
    switch_generation(const QString& generation_id) {

        // Initialize the lists for output and full error
        QStringList output;
        QStringList full_error;
        bool success;

        std::tie(success, output, full_error) = exec_bash(QStringLiteral("nix-env --switch-generation %1").arg(generation_id));

        // Return the two lists as a tuple + success bool
        return {success, output, full_error};
    }

    std::tuple<bool, QStringList, QStringList>
    delete_generation(const QString& generation_id) {

        // Initialize the lists for output and full error
        QStringList output;
        QStringList full_error;
        bool success;

        std::tie(success, output, full_error) = exec_bash(QStringLiteral("nix-env --delete-generations %1").arg(generation_id));

        // Return the two lists as a tuple + success bool
        return {success, output, full_error};
    }

    std::tuple<bool, QStringList, QStringList>
    delete_old_generations() {

        // Initialize the lists for output and full error
        QStringList output;
        QStringList full_error;
        bool success;

        std::tie(success, output, full_error) = exec_bash(QStringLiteral("nix-env --delete-generations old"));

        // Return the two lists as a tuple + success bool
        return {success, output, full_error};
    }
}


namespace NixChannel {

    std::tuple<bool, QStringList, QStringList>
    update_channels() {

        // Initialize the lists for output and full error
        QStringList output;
        QStringList full_error;
        bool success;

        std::tie(success, output, full_error) = exec_bash(QStringLiteral("nix-channel --update"));

        // Return the two lists as a tuple + success bool
        return {success, output, full_error};
    }


    std::tuple<bool, QStringList, QStringList>
    add_channel(const QString& url, const QString& name) {

        // Initialize the lists for output and full error
        QStringList output;
        QStringList full_error;
        bool success;

        std::tie(success, output, full_error) = exec_bash(QStringLiteral("nix-channel --add %1 %2").arg(url, name));

        // Return the two lists as a tuple
        return {success, output, full_error};
    }

    std::tuple<bool, QStringList, QStringList>
    remove_channel(const QString& name) {

        // Initialize the lists for output and full error
        QStringList output;
        QStringList full_error;
        bool success;

        std::tie(success, output, full_error) = exec_bash(QStringLiteral("nix-channel --remove %1").arg(name));

        // Return the two lists as a tuple
        return {success, output, full_error};
    }

    std::tuple<bool, QStringList, QStringList>
    list_channels() {

        // Initialize the lists for output and full error
        QStringList output;
        QStringList processed_output;
        QStringList full_error;
        bool success;

        std::tie(success, output, full_error) = exec_bash(QStringLiteral("nix-channel --list"));

        if (output.isEmpty()) {
            full_error << QStringLiteral("No Channels Found!");
            return {false, {}, full_error};
        }

        // Process the output into a structured format
        // Example input lines:
        //   name url
        // Output JSON-like:
        //   {"name": "name", "url": "url"}
        for (const QString &line : output) {
            QString trimmed = line.trimmed();
            if (trimmed.isEmpty()) continue;

            // Split into at most 2 parts: name and url (url may contain colons/slashes)
            QStringList parts = trimmed.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            if (parts.size() < 2) continue;

            QString name = parts.at(0);
            QString url = parts.mid(1).join(' '); // join remainder as url

            QString json_entry = QStringLiteral("{\"name\": \"%1\", \"url\": \"%2\"}")
                                    .arg(name, url);
            processed_output << json_entry;
        }

        return {success, processed_output, full_error};
    }
}