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



#ifndef OPENPROCESS_H
#define OPENPROCESS_H

#include <QProcess>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QRegExp>

/**
 * @brief Executes a shell command and captures its standard output and standard error.
 *
 * This function utilizes `openProcess` to run the specified shell command, capturing
 * both the standard output and standard error in separate vectors. It evaluates the
 * exit code to determine the success of the command execution. If the command fails,
 * an error message is appended to the standard error vector.
 *
 * @note This implementation assumes that `openProcess` is defined elsewhere and handles
 * the actual process creation and output capturing.
 *
 * @param command The shell command string to execute.
 * @return A tuple containing a boolean indicating success, a vector of strings for
 * the command's stdout, and a vector of strings for the command's stderr.
 */
std::tuple<bool, QStringList, QStringList>
exec_bash(const QString& command);

#endif // OPENPROCESS_H