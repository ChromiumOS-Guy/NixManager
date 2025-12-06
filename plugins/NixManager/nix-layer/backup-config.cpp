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

#include "backup-config.h"


// Helper for 'rm -f' equivalent
std::tuple<bool, QString> remove_file_qt(const QString &path) {
    // QFile::remove returns true if the file was removed, false otherwise.
    // QFile does not provide std::error_code; use errno for additional info when needed.
    if (!QFile::exists(path)) {
        // Mimic "rm -f": treat non-existence as success.
        return {true, QString("SUCCESS: File '%1' did not exist, no removal needed.").arg(path)};
    }

    if (QFile::remove(path)) {
        return {true, QString("SUCCESS: File '%1' removed.").arg(path)};
    }

    // QFile::remove failed although the file existed. Try to extract an errno if available.
    int err = errno; // may reflect last OS error
    if (err == ENOENT) {
        // File disappeared between exists() and remove(); treat as success (rm -f behavior).
        return {true, QString("SUCCESS: File '%1' did not exist, no removal needed.").arg(path)};
    }

    QString errMsg = QString("Failed to remove file '%1': %2").arg(path, QString::fromLocal8Bit(strerror(err ? err : 0)));
    return {false, errMsg};
}


// Helper for 'cp' equivalent (simple copy, fails if destination does not exists)
std::tuple<bool, QString> copy_file_qt(const QString &source,
                                       const QString &destination,
                                       bool overwrite = false) {
    QFile src(source);
    if (!src.exists()) {
        return {false, QString("Failed to copy file from '%1' to '%2': source does not exist")
                         .arg(source, destination)};
    }

    if (QFile::exists(destination)) {
        if (!overwrite) {
            return {false, QString("Failed to copy file from '%1' to '%2': destination exists and overwrite is false")
                             .arg(source, destination)};
        }
        if (!QFile::remove(destination)) {
            return {false, QString("Failed to copy file from '%1' to '%2': cannot remove existing destination")
                             .arg(source, destination)};
        }
    }

    if (!src.copy(destination)) {
        return {false, QString("Failed to copy file from '%1' to '%2': copy operation failed")
                         .arg(source, destination)};
    }

    return {true, QStringLiteral("SUCCESS: File copied.")};
}

// Helper for 'mkdir -p' equivalent
std::tuple<bool, QString> create_directories_qt(const QString &path) {
    QDir dir;
    // QDir::mkpath returns true if the path was successfully created or already exists.
    bool ok = dir.mkpath(path);
    if (!ok) {
        return {false, QString("Failed to create directories '%1': mkpath failed").arg(path)};
    }

    // Verify that the path exists and is a directory.
    QFileInfo info(path);
    if (!info.exists()) {
        return {false, QString("Failed to create directories '%1': path does not exist after mkpath").arg(path)};
    }
    if (!info.isDir()) {
        return {false, QString("Failed to create directories: '%1' exists but is not a directory after mkpath call").arg(path)};
    }

    return {true, QStringLiteral("SUCCESS: Directories created or already existed.")};
}

// Mimics filecmp.cmp(path1, path2, shallow=False)
// Returns true if files are identical, false otherwise.
bool are_files_identical_qt(const QString &p1, const QString &p2) {
    QFileInfo info1(p1);
    QFileInfo info2(p2);

    // Both must exist and be regular files
    if (!info1.exists() || !info1.isFile() || !info2.exists() || !info2.isFile())
        return false;

    // Quick size check
    if (info1.size() != info2.size())
        return false;

    // Open files for binary reading
    QFile f1(p1);
    QFile f2(p2);
    if (!f1.open(QIODevice::ReadOnly) || !f2.open(QIODevice::ReadOnly))
        return false;

    // Compare in chunks for efficiency
    const qint64 bufSize = 64 * 1024; // 64 KB
    QByteArray b1, b2;
    while (!f1.atEnd() && !f2.atEnd()) {
        b1 = f1.read(bufSize);
        b2 = f2.read(bufSize);
        if (b1 != b2) {
            return false;
        }
    }

    // Both should be at end (sizes were equal so this should hold)
    return f1.atEnd() && f2.atEnd();
}

// Equivalent to Python's _backup_config_file.
std::tuple<bool, QString> backup_config_file(const QString& filename) {
    QString backup_path = filename + ".backup";

    // Use QFileInfo for file system checks
    bool src_is_file = QFileInfo(filename).isFile();
    bool backup_is_file = QFileInfo(backup_path).isFile();

    if (src_is_file) {
        // Case A: Source file exists and is a regular file. Standard backup procedure.
        if (backup_is_file) {
            // Backup exists and is a regular file. Compare.
            if (are_files_identical_qt(filename, backup_path)) {
                return {true, "SUCCESS: Backup is already identical to current config."};
            } else {
                // Backup exists but is different. Overwrite it.
                auto [rm_success, rm_msg] = remove_file_qt(backup_path);
                if (!rm_success) {
                    return {false, "Failed to remove old backup: " + rm_msg};
                }
                auto [cp_success, cp_msg] = copy_file_qt(filename, backup_path);
                return {cp_success, cp_success ? "SUCCESS: Updated backup created." : "Failed to copy new backup: " + cp_msg};
            }
        } else if (QFileInfo(backup_path).exists()) {
            // Something exists at backup_path, but it's NOT a regular file (e.g., a directory, symlink).
            // This is an inconsistent state; treat as failure for backup.
            return {false, "Error: Cannot create backup. Path '" + backup_path + "' exists but is not a regular file. Please investigate/remove it manually."};
        }
        else {
            // No backup file exists at backup_path. Create a new one.
            auto [cp_success, cp_msg] = copy_file_qt(filename, backup_path);
            return {cp_success, cp_success ? "SUCCESS: New backup created." : "Failed to create new backup: " + cp_msg};
        }
    } else {
        // Case B: Source file does NOT exist or is not a regular file.
        if (QFileInfo(filename).exists() && !src_is_file) {
            // Source path exists but is not a regular file (e.g., a directory).
            return {false, "Error: Source path '" + filename + "' exists but is not a regular file. Cannot create backup."};
        }
        if (backup_is_file) {
            // If backup exists (and is a regular file) but source doesn't (or isn't a file), FAIL the backup operation.
            return {false, "Error: Source file '" + filename + "' does not exist (or is not a regular file), but a stale backup '" + backup_path + "' was found. Backup operation aborted."};
        } else if (QFileInfo(backup_path).exists() && !backup_is_file) {
             // If something exists at backup_path, but it's not a regular file, and source also doesn't exist/isn't a file.
             // This is an odd state. Let's make it a failure to be safe.
            return {false, "Error: Source file '" + filename + "' does not exist. Backup path '" + backup_path + "' exists but is not a regular file. Backup operation aborted."};
        }
        else {
            // Neither source nor backup exist (or are regular files). No backup action needed, unsuccessful "no-op".
            return {false, "Error: Source file does not exist, and no regular file backup was present."};
        }
    }
}

// Equivalent to Python's _restore_config_file
std::tuple<bool, QString> restore_config_file(const QString& filename) {
    QString backup_path = filename + ".backup";

    // Use QFileInfo for file system checks
    bool current_file_is_file = QFileInfo(filename).isFile();
    bool backup_is_file = QFileInfo(backup_path).isFile();

    if (!backup_is_file) {
        // Case A: Backup file does NOT exist or is not a regular file.
        if (QFileInfo(backup_path).exists() && !backup_is_file) {
            // Something exists at backup_path but it's not a regular file (e.g., directory, symlink).
            return {false, "Error: Cannot restore. Backup path '" + backup_path + "' exists but is not a regular file. Please investigate/remove it manually."};
        }
        if (current_file_is_file) {
            // Config exists (and is a regular file), but no valid backup file was found. FAIL the restore.
            return {false, "Error: Configuration file '" + filename + "' exists, but no corresponding regular file backup '" + backup_path + "' was found. Restore operation aborted."};
        } else if (QFileInfo(filename).exists() && !current_file_is_file) {
            // Current config path exists, but is not a regular file. And no backup.
            return {false, "Error: Configuration path '" + filename + "' exists but is not a regular file, and no backup was present. No restoration performed."};
        }
        else {
            // Neither config nor backup exist (or are regular files). No restore action needed.
            return {false, "Error: Neither configuration file nor its backup were found. No restoration can be done."};
        }
    }

    // From this point on, backup_is_file is true.

    if (current_file_is_file && are_files_identical_qt(filename, backup_path)) {
        return {true, "SUCCESS: Configuration file is already identical to its backup."};
    } else {
        // Ensure the directory for filename exists before copying
        QString dir = QFileInfo(filename).path();
        // Check if directory exists before creating
        if (!QFileInfo(dir).isDir()) {
            auto [dir_created, dir_msg] = create_directories_qt(dir); // Using C++ filesystem
            if (!dir_created) {
                return {false, "Error creating directory for restoration: " + dir_msg};
            }
        } else if (!QFileInfo(dir).exists()) {
            // Path exists but is not a directory
            return {false, "Error: Cannot restore. Parent path '" + dir + "' exists but is not a directory."};
        }

        // Overwrite the current file with the backup
        auto [cp_success, cp_msg] = copy_file_qt(backup_path, filename, true); // Using C++ filesystem
        if (cp_success) {
            return {true, "SUCCESS: Configuration file restored from backup."};
        } else {
            return {false, "Error restoring from backup: " + cp_msg};
        }
    }
}

QString get_config_path() {
    bool success;
    QStringList output;
    QStringList full_error;

    std::tie(success, output, full_error) = exec_bash(QString("echo $HOME"));

    if (!output.isEmpty()) {
        QString userdir = output.first().trimmed(); // Remove trailing newline
        if (userdir.isEmpty()) {
            qDebug() << "Error: $HOME is empty. Cannot determine config path.";
            return QString();
        }
        // default user return: /home/phablet/.config/home-manager/home.nix
        QString config_path = userdir + "/.config/home-manager/home.nix";

        if (!QFile::exists(config_path) || !QFile(config_path).open(QIODevice::ReadOnly)) {
            // File doesn't exist or can't be opened as a regular file
            return QString(); // return empty string as per brief in header file 
        }

        return config_path;
    } else {
        qDebug() << "Error: Could not determine userdir using 'echo $HOME'." << full_error;
        return QString();
    }
}
