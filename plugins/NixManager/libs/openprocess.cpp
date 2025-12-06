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
 * made by ChromiumOS-Guy (https://github.com/ChromiumOS-Guy)
 */

#include "openprocess.h"  


std::tuple<bool, QStringList, QStringList>
exec_bash(const QString& command) {
    QStringList output;
    QStringList full_error;
    QString cmd = QString("source %1/.profile && %2")
                  .arg(QString::fromUtf8(qgetenv("HOME")))
                  .arg(command);

    QProcess proc;
    proc.start("/bin/bash", QStringList() << "-c" << cmd);
    if (!proc.waitForStarted()) {
        full_error << QString("Failed to start process: %1").arg(proc.errorString());
        return {false, output, full_error};
    }

    // Block until finished
    proc.waitForFinished(-1);

    // Read and split stdout into lines
    QByteArray stdoutData = proc.readAllStandardOutput();
    if (!stdoutData.isEmpty()) {
        const QString stdoutText = QString::fromUtf8(stdoutData);
        output.append(stdoutText.split(QRegExp("\r?\n"), QString::SkipEmptyParts)); // remove trailing line
    }

    // Read and split stderr into lines
    QByteArray stderrData = proc.readAllStandardError();
    if (!stderrData.isEmpty()) {
        const QString stderrText = QString::fromUtf8(stderrData);
        full_error.append(stderrText.split(QRegExp("\r?\n"), QString::SkipEmptyParts));
    }

    int exitCode = proc.exitCode();
    bool success = (proc.exitStatus() == QProcess::NormalExit && exitCode == 0);

    if (!success) {
        full_error << QString("'%1' exited with error code: %2").arg(command, QString::number(exitCode));
    }

    return {success, output, full_error};
}