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


#include "nix-config.h"

// Function to trim leading/trailing whitespace from a string
QString trim(const QString& str) {
    size_t first = str.toStdString().find_first_not_of(" \t\n\r\f\v");
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.toStdString().find_last_not_of(" \t\n\r\f\v");
    return QString::fromStdString(str.toStdString().substr(first, (last - first + 1)));
}

// Function to get leading whitespace length
int getLeadingWhitespaceLength(const QString& str) {
    size_t first = str.toStdString().find_first_not_of(" \t\n\r\f\v");
    if (std::string::npos == first) {
       return str.length();
    }
    return first;
}


QStringList readFile(const QString &path) {
    QFile file(path);
    QStringList lines;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return lines; // empty on error
    QTextStream in(&file);
    while (!in.atEnd()) {
        lines.append(in.readLine());
    }
    file.close();
    return lines;
}


bool writeFile(const QString &path, const QStringList &lines) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;
    QTextStream out(&file);
    for (const QString &line : lines) {
        out << line << '\n'; // this single line is responsible for config file working DO NOT CHANGE or you will wreck all newline char
    }
    file.close();
    return true;
}

namespace FileProcessing {

    // Structure to represent a package block    // defined in header!
    // struct PackageBlock {
    //     std::string package_type;
    //     int start_line;
    //     int end_line;
    //     int start_indent;
    // };

    /**
     * @brief Generates a list of packages from a configuration file.
     *
     * The function reads a configuration file, identifies the '.packages' block,
     * and extracts the package type, start line, and end line. It returns a list
     * of PackageBlock structs, where each struct represents a .packages block.
     *
     * @param filename The path to the configuration file.
     * @return A list of PackageBlock structs, where each struct represents a package.
     */
   QVector<PackageBlock> process_file(const QString& filename) {
        QVector<PackageBlock> package_blocks;
        PackageBlock current_package_block;
        current_package_block.package_type = "";
        current_package_block.start_line = -1;
        current_package_block.end_line = -1;
        current_package_block.start_indent = -1;

        try {
            QStringList lines = readFile(filename);
            int i = 0;
            for (const QString &line : lines) {
                QString stripped_line = trim(line);
                if (current_package_block.start_line == -1 && line.contains(".packages") && !stripped_line.startsWith('#')) { // must not be a comment, must have a ".packages" and be on start of block.
                    current_package_block.start_indent = getLeadingWhitespaceLength(line);
                    qsizetype pos = line.indexOf(".packages");
                    if (pos == -1) continue;
                    QString package_type = line.left(pos).trimmed();
                    current_package_block.package_type = package_type;
                    current_package_block.start_line = i + 1; // so line after home.packages = [
                } else if (current_package_block.start_line == -1 && line.contains(".systemPackages") && !stripped_line.startsWith('#')) { // obsulete check useful only in case configuration.nix is used.
                    current_package_block.start_indent = getLeadingWhitespaceLength(line);
                    current_package_block.package_type = "system";
                    current_package_block.start_line = i + 1; // so line after home.packages = [
                } else if (current_package_block.start_line != -1 && !stripped_line.startsWith('#') && // must not be a comment/on the same indent as start, must have a "];" and not be on start of block.
                           getLeadingWhitespaceLength(line) == current_package_block.start_indent &&
                           line.contains("];")) {
                    current_package_block.end_line = i - 1; // so line before ];
                    package_blocks.push_back(current_package_block);
                    current_package_block.package_type = ""; //reset
                    current_package_block.start_line = -1; // taking advantage of the fact -1 is not valid line number for conditional checks with minimal logic.
                    current_package_block.end_line = -1; // set to -1 so there will be an error if it is not set.
                    current_package_block.start_indent = -1; // set to -1 so there will be an error if it is not set.

                }
                i++;
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception occurred! , error is " << e.what();
        }

        return package_blocks;
    }

} // namespace FileProcessing

namespace PackageChecks {

    /**
     * @brief Checks if there are any duplicate package block types in the given list, and that package_blocks has data in it.
     *
     * @param package_blocks A list of package blocks, where each block is a PackageBlock struct containing package block information.
     * @return True if there are any problems with package_blocks list, False otherwise.
     */
    bool check_package_blocks(const QVector<FileProcessing::PackageBlock> &package_blocks) {
        if (package_blocks.isEmpty()) {
            qDebug() << "no package blocks found in configuration file\n";
            return true;
        }

        QSet<QString> seen_types;
        for (const auto &package_block : package_blocks) {
            const QString &type = package_block.package_type;
            if (seen_types.contains(type)) {
                qDebug() << "Error: Duplicate package block of type '" << type << "' this is not supported.\n";
                return true;
            }
            seen_types.insert(type);
        }
        return false;
    }

} // namespace PackageChecks

namespace PackageOperations {

    /**
     * @brief Reads and extracts packages from a configuration file based on the specified packages type.
     *
     * @param filename The path to the configuration file.
     * @param package_type The type of packages to extract (e.g., 'home', 'system').
     * @return A list of packages extracted from the configuration file, filtered to exclude comments, newlines, and unsupported syntax.
     */
    QStringList read_packages(const QString& filename, const QString& package_type) {
        QStringList packages;
        QVector<FileProcessing::PackageBlock> package_blocks = FileProcessing::process_file(filename);

        if (PackageChecks::check_package_blocks(package_blocks)) {
            return packages;
        }

        try {
            for (const auto& package_block : package_blocks) {
                if (package_block.package_type == package_type) {
                    QStringList lines = readFile(filename);
                    int current_line_num = 0;
                    for (const QString &line : lines) {
                        if (current_line_num >= package_block.start_line && current_line_num <= package_block.end_line) { // must be withing specified limits
                            QString stripped_line = trim(line);
                            if (!stripped_line.startsWith('#') && // must not be a comment, a context (), a bracket [] or an empty line.
                                !stripped_line.contains('(') &&
                                !stripped_line.contains(')') &&
                                !stripped_line.contains('[') &&
                                !stripped_line.contains(']') &&
                                !stripped_line.isEmpty()) {
                                packages.append(stripped_line);
                            }
                        }
                        current_line_num++;
                    }
                }
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception occurred! , error is " << e.what();
        }

        return packages;
    }

    /**
     * @brief Adds packages to a configuration file or overwrites them.
     *
     * @param filename The path to the configuration file.
     * @param packages A list of packages to add to the file.
     * @param package_type The type of package block to add to (e.g., 'home', 'system').
     * @param overwrite Whether to overwrite existing packages in the file. Defaults to false.
     * @return Packages added in list data type.
     */
    QStringList add_packages(const QString& filename, QStringList packages, const QString& package_type, bool overwrite) {
        QStringList processed_packages;
        for (const auto& package : packages) {
            if (package.startsWith("nixpkgs")) { // startsWith("nixpkgs")
                processed_packages.append(QStringLiteral("pkgs") + package.mid(QStringLiteral("nixpkgs").length())); // if a package starts with nixpkgs.name it will become pkgs.name
            } else {
                processed_packages.append(package); // if a package has starts with *.name will leave it be and assume user knows what he is doing or get served an error by later error catching in nix-wrapper.cpp
            }
        }
        packages = processed_packages;

        if (packages.isEmpty()) {
            packages.append("#empty"); // Add a placeholder to prevent syntax issues
        }

        QVector<FileProcessing::PackageBlock> package_blocks = FileProcessing::process_file(filename);
        QStringList existing_packages = overwrite ? QStringList() : read_packages(filename, package_type);

        if (PackageChecks::check_package_blocks(package_blocks)) {
            return QStringList();
        }

        for (const auto& package : existing_packages) {
            packages.append(package);
        }

        // Remove duplicates and maintain order
        QSet<QString> seen_packages_set;
        QStringList unique_packages;
        for (const QString &pkg : packages) {
            if (!seen_packages_set.contains(pkg)) {
                unique_packages.append(pkg);
                seen_packages_set.insert(pkg);
            }
        }
        packages = unique_packages;

        try {
            for (const auto& package_block : package_blocks) {
                if (package_block.package_type == package_type) {
                    QStringList lines = readFile(filename);

                    QStringList new_package_lines;
                    for (const auto& pkg : packages) {
                        new_package_lines.append(QString(QChar(' ')).repeated(package_block.start_indent + 2) + pkg); // make indent
                    }

                    // insert empty line at front
                    new_package_lines.insert(0, QString());

                    // Replace the lines in the package block
                    // std::cerr << "start of block " << package_block.start_line << " end of block " << package_block.end_line << std::endl;
                    if (package_block.start_line >= 0 && static_cast<qsizetype>(package_block.end_line) < lines.size()) { // basic checks to validate start/end line were correctly read and make sense.
                        if (package_block.start_line < package_block.end_line) { // if there is more then one line between true end/start of block (which is offset by one when we get it hence true end/start is syntax of the array we are typing into).
                            lines = lines.mid(0, package_block.start_line) + new_package_lines + lines.mid(package_block.start_line + package_block.end_line - package_block.start_line); // erase extra lines (so we won't end up with duplicates) and insert new packages
                        } else if (package_block.start_line == package_block.end_line) {
                            lines = lines.mid(0, package_block.start_line) + new_package_lines + lines.mid(package_block.start_line); // insert new packages
                        }
                    }

                    bool success = writeFile(filename, lines);
                    if (!success) {
                        qDebug() << "could not write to file, path or perms incorrect! " << filename;
                        return QStringList();
                    }
                }
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception occurred! , error is " << e.what();
        }
        return packages;
    }

    /**
     * @brief Deletes a list of packages from a configuration file.
     *
     * @param filename The name of the file from which to delete packages.
     * @param packages A list of packages to delete.
     * @param package_type The type of package to delete. If not specified, all package types will be checked. Defaults to empty string.
     * @return A list of packages that were successfully deleted.
     */
    QStringList delete_packages(const QString& filename, const QStringList& packages, const QString& package_type) {
        QStringList deleted_packages;
        QVector<FileProcessing::PackageBlock> package_blocks = FileProcessing::process_file(filename);

        if (PackageChecks::check_package_blocks(package_blocks)) {
            return QStringList();
        }

        QSet<QString> seen_types;
        if (package_type.isEmpty()) {
            for (const auto &package_block : package_blocks)
                seen_types.insert(package_block.package_type);
        } else {
            seen_types.insert(package_type);
        }

        for (const auto& type : seen_types) {
            QStringList existing_packages = read_packages(filename, type);
            QStringList packages_to_delete;

            // find intersection (packages to delete)
            for (const auto& pkg_to_del : packages) {
                if (existing_packages.contains(pkg_to_del)) {
                    packages_to_delete.append(pkg_to_del);
                }
            }

            // build updated_packages = existing_packages - packages_to_delete
            QStringList updated_packages;
            for (const auto& existing_pkg : existing_packages) {
                if (!packages_to_delete.contains(existing_pkg)) { // we skip adding packages we don't want
                    updated_packages.append(existing_pkg);
                }
            }

            add_packages(filename, updated_packages, type, true); // then we use override feature on add_packages to "delete" the packages we skiped over.

            deleted_packages.append(packages_to_delete);
        }
        return deleted_packages;
    }

} // namespace PackageOperations