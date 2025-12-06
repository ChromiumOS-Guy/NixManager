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


// nix-config.h
#ifndef NIX_CONFIG_H
#define NIX_CONFIG_H

#include <QString>
#include <QStringList>
#include <QVector>
#include <QSet>
#include <QDebug>
#include <QFile>
#include <QTextStream>

// Function declarations

/**
 * @brief Trims leading and trailing whitespace from a string.
 *
 * @param str The input string to be trimmed.
 * @return A new string with leading and trailing whitespace removed.
 */
std::string trim(const std::string& str);

/**
 * @brief Calculates the number of leading whitespace characters in a string.
 *
 * @param str The input string.
 * @return The count of leading whitespace characters.
 */
int getLeadingWhitespaceLength(const std::string& str);

namespace FileProcessing {
    /**
     * @brief Represents a block of packages within the configuration file.
     */
    struct PackageBlock {
        QString package_type;  ///< The type of package block (e.g., "packages", "programs").
        int start_line;            ///< The 0-indexed starting line number of the block in the file.
        int end_line;              ///< The 0-indexed ending line number of the block in the file.
        int start_indent;          ///< The leading whitespace length of the block's start line.
    };

    /**
     * @brief Processes a configuration file to identify and extract package blocks.
     *
     * This function reads the specified file line by line to determine the
     * boundaries and types of different package definition blocks (e.g., `environment.systemPackages`,
     * `home.packages`). It's crucial for understanding the structure of the Nix
     * configuration for subsequent package operations.
     *
     * @param filename The full path to the Nix configuration file (e.g., "home.nix").
     * @return A QVector of PackageBlock structs, each describing a found package block.
     */
    QVector<PackageBlock> process_file(const QString& filename);
} // namespace FileProcessing

namespace PackageChecks {
    /**
     * @brief Performs consistency checks on the identified package blocks.
     *
     * This function validates the integrity and expected structure of the package blocks
     * extracted by `FileProcessing::process_file`. It might check for overlapping blocks,
     * correct nesting, or other structural invariants to ensure reliable package operations.
     *
     * @param package_blocks A vector of PackageBlock structs to be checked.
     * @return True if all package blocks pass the checks, false otherwise.
     */
    bool check_package_blocks(const std::vector<FileProcessing::PackageBlock>& package_blocks);
} // namespace PackageChecks

namespace PackageOperations {
    /**
     * @brief Reads packages of a specific type from the configuration file.
     *
     * This function parses the content of a designated package block within the
     * configuration file and extracts the list of package names.
     *
     * @param filename The full path to the Nix configuration file.
     * @param package_type The type of package block to read (e.g., "system", "home", etc., mapping to `PackageBlock::package_type`).
     * @return A QStringList, where each QString is a package name.
     */
    QStringList read_packages(const QString& filename, const QString& package_type);

    /**
     * @brief Adds new packages to a specified block in the configuration file.
     *
     * This function inserts new package entries into the relevant package block
     * within the configuration file. It can either append to the existing list
     * or replace it entirely based on the `overwrite` flag.
     *
     * @param filename The full path to the Nix configuration file.
     * @param packages A vector of strings representing the package names to add.
     * @param package_type The type of package block to modify (e.g., "system", "home").
     * @param overwrite If true, existing packages in the block are replaced by the new list. If false, new packages are added. Defaults to false.
     * @return A QStringList representing the *new* state of packages in that block after the operation.
     */
    // std::vector<std::string> add_packages(const std::string& filename, std::vector<std::string> packages, const std::string& package_type, bool overwrite = false);
    QStringList add_packages(const QString& filename, QStringList packages, const QString& package_type, bool overwrite = false);

    /**
     * @brief Deletes specified packages from a block in the configuration file.
     *
     * This function removes one or more package entries from a specific package
     * block within the configuration file.
     *
     * @param filename The full path to the Nix configuration file.
     * @param packages A vector of strings representing the package names to delete.
     * @param package_type The type of package block to modify (e.g., "system", "home"). If empty, attempts to delete from all found blocks.
     * @return A QStringList representing the *new* state of packages in the modified block(s) after the operation.
     */
    // std::vector<std::string> delete_packages(const std::string& filename, const std::vector<std::string>& packages, const std::string& package_type = "");
    QStringList delete_packages(const QString& filename, const QStringList& packages, const QString& package_type = QString());
} // namespace PackageOperations

#endif // NIX_CONFIG_H