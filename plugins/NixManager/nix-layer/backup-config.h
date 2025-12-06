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

#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "../libs/openprocess.h"
#include <QString>
#include <QFile>
#include <tuple>
#include <cerrno>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QDebug>

/**
 * @brief Compares the content of two files byte by byte.
 *
 * This function checks if two specified files exist, are regular files,
 * have the same size, and then compares their contents to determine if they
 * are identical.
 *
 * @param p1 The path to the first file.
 * @param p2 The path to the second file.
 * @return True if the files are identical in content and size, false otherwise
 * (including if files don't exist, are not regular files, or cannot be opened).
 */
bool are_files_identical_qt(const QString &p1, const QString &p2);

/**
 * @brief Creates a backup of a specified configuration file.
 *
 * This function attempts to create a backup of the `filename` by copying it
 * to `filename.backup`. If a backup file already exists, it checks if the
 * source file and the existing backup are identical.
 * - If they are identical, no action is taken, and it reports success.
 * - If they are different, the existing backup is replaced with the new
 * version from the source file.
 *
 * @param filename The absolute path to the configuration file to be backed up.
 * @return A tuple where:
 * - The first element (bool) is `True` if the backup operation was
 * successful or if the files were already identical; `False` otherwise.
 * - The second element (QString) is a success message ("SUCCESS") or an
 * error message detailing the failure.
 */
std::tuple<bool, QString> backup_config_file(const QString& filename);

/**
 * @brief Restores a configuration file from its backup.
 *
 * This function attempts to restore the specified `filename` by copying its
 * corresponding backup file (`filename.backup`) back to the original `filename` path.
 * It performs several checks:
 * - Verifies that the backup file (`filename.backup`) exists. This is crucial for restoration.
 * - If a backup exists, it then checks if the current `filename` (if it exists)
 * and the `filename.backup` are already identical. If so, no action is taken.
 * - If the files are different, or if the original `filename` doesn't exist,
 * the current `filename` is either overwritten or created with the content
 * of `filename.backup`.
 *
 * @param filename The absolute path to the configuration file to be restored.
 * This is the target path where the backup will be copied to.
 * @return A tuple where:
 * - The first element (bool) is `True` if the restore operation was
 * successful or if the files were already identical; `False` otherwise.
 * - The second element (QString) is a success message ("SUCCESS") or an
 * error message detailing the failure.
 */
std::tuple<bool, QString> restore_config_file(const QString& filename);

/**
 * @brief Determines and verifies the default configuration path for home-manager.
 *
 * This function constructs the expected path to the home-manager configuration file
 * at `~/.config/home-manager/home.nix` by dynamically getting the $HOME
 * using the `echo $HOME` command via `openProcess`. It then checks if a file actually
 * exists at this path.
 *
 * @return The absolute path to the home-manager configuration file if it
 * exists and is a regular file. Returns an empty `std::string` if the
 * file is not found at the expected location or if the username cannot be
 * determined. A message indicating the missing file path or an error
 * will be printed to standard output/error if applicable.
 */
QString get_config_path();

#endif // CONFIG_MANAGER_H