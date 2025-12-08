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
 * ported by ChromiumOS-Guy (https://github.com/ChromiumOS-Guy), from crackle.
 * made by Fuseteam (https://gitlab.com/tuxecure), in crackle.
 */

#ifndef NIX_SETUP_H
#define NIX_SETUP_H

#include "libs/openprocess.h"  // Include openProcess function declaration

#include <QString>
#include <QStringList>
#include <QDir>
#include <tuple>
#include <QFileInfo>


namespace SetupNixHomeManager {
    /**
    * @brief Installs Nix and Home Manager for the current user.
    *
    * This function ensures temporary helper tools (curl, xz) are installed, then
    * runs a shell script to install Nix and the Home Manager channel/installer.
    * It supports installing a specific NixOS release channel when a version
    * string is provided; otherwise it uses the (unstable/master) channel flow.
    * Temporary files are removed after the install attempt regardless of success.
    *
    * Behavior:
    * - If version != "":
    *   - Installs Nix (no-daemon, no-channel-add), sets up nixpkgs channel for the
    *     provided version, and adds Home Manager channel from the matching release.
    * - If version == "":
    *   - Installs Nix (no-daemon), updates channels, and adds Home Manager from
    *     the repository master (unstable) branch.
    * - Uses SetupCurlXz::install_curlxz(extraction_path) to provision temporary
    *   curl/xz tools in extraction_path before running the shell command.
    * - Executes the generated multi-line shell command via exec_sh(command).
    * - Calls SetupCurlXz::remove_curlxz(extraction_path) to clean temporary files.
    * - Aggregates stdout/stderr from each step into returned vectors.
    *
    * @param version The NixOS release version string to target (e.g., "25.05").
    *                If empty, the function uses the default/unstable flow.
    * @return std::tuple<bool, QStringList, QStringList>
    *   - first: overall success of the shell command (true if the install command
    *     succeeded; false otherwise). Note: failures in provisioning cause
    *     an immediate false return with corresponding errors.
    *   - second: aggregated stdout output lines from all steps (install_curlxz,
    *     exec_sh, remove_curlxz).
    *   - third: aggregated stderr/error message lines from all steps plus any
    *     added contextual error messages (e.g., failures to install or clean temp files).
    */
    std::tuple<bool, QStringList, QStringList>
    install_nix_home_manager(const QString& version);

    /**
    * @brief Uninstalls Nix and Home Manager from the user system.
    *
    * This function runs a hard-removal shell sequence to delete Nix store contents
    * and Home Manager user state. It executes a bash heredoc that:
    *  - recursively sets permissive permissions on /nix contents,
    *  - removes all files under /nix,
    *  - removes Home Manager user directories ($HOME/.config/home-manager,
    *    $HOME/.local/share/home-manager),
    *  - removes various Nix user state and caches ($HOME/.nix-profile,
    *    $HOME/.nix-defexpr, $HOME/.nix-channels, $HOME/.nix-command-errors.log,
    *    $HOME/.cache/nix, $HOME/.local/share/nix, $HOME/.local/state/nix).
    *
    * The command is executed via exec_sh and the function returns a tuple with
    * (success, output_lines, error_lines). Note: the function currently returns
    * `false` for the overall success value unconditionally (matching the source).
    *
    * @return std::tuple<bool, QStringList, QStringList>
    *   - bool: overall success flag (currently always false in this implementation).
    *   - std::vector<std::string>: collected stdout/stderr output lines from the shell execution.
    *   - std::vector<std::string>: collected error lines produced by exec_sh.
    */
    std::tuple<bool, QStringList, QStringList>
    uninstall_nix_home_manager();
}

// namespace SetupCurlXz {
//     /**
//     * @brief Downloads and extracts curl and xz-utils binaries to a temporary directory.
//     *
//     * Runs an isolated bash script that prepares APT state/cache/config directories (using
//     * XDG fallbacks), downloads the curl and xz-utils .deb packages without installing them,
//     * and extracts their /usr/bin contents into the specified output directory.
//     *
//     * The function constructs and executes the script, capturing stdout/stderr lines and
//     * returning success status plus collected output and error messages.
//     *
//     * @param extraction_path Absolute Path (runs in bash so ENV variables can be used) to the output directory where extracted binaries will be placed.
//     *                       The function creates the directory if it does not exist.
//     * @return std::tuple<bool, QStringList, QStringList>
//     *         - bool: true if the script completed successfully, false on failure.
//     *         - std::vector<std::string>: collected stdout output lines from the script.
//     *         - std::vector<std::string>: collected stderr/error lines from the script.
//     */
//     std::tuple<bool, QStringList, QStringList>
//     install_curlxz(const QString& extraction_path);

//     /**
//     * @brief Remove temporary curl/xz files and apt state/cache created in install_curlxz.
//     *
//     * Runs a shell script that deletes the specified OUTDIR (extraction_path) and
//     * the resolved APT configuration/state/cache directories:
//     *   - APTCACHE (defaults to XDG_CACHE_HOME or $HOME/.cache/apt)
//     *   - APTSTATE (defaults to XDG_STATE_HOME or $HOME/.local/state/apt)
//     *   - APTCONFIG (defaults to XDG_CONFIG_HOME or $HOME/.config/apt)
//     *
//     * The function executes the script via exec_sh and returns the result tuple:
//     *   - success (bool): whether the removal command succeeded
//     *   - output (vector<string>): stdout lines from the command
//     *   - full_error (vector<string>): stderr or error lines from the command
//     *
//     * @param extraction_path Path used as OUTDIR for extracted binaries (will be removed).
//     * @return std::tuple<bool, QStringList, QStringList>
//     */
//     std::tuple<bool, QStringList, QStringList>
//     remove_curlxz(const QString& extraction_path);
// }

namespace DetectNixHomeManager {
    /**
    * @brief Detects and validates a Nix + Home Manager installation for the current user.
    *
    * Runs a shell check for the `home-manager` command, obtains $HOME, and verifies
    * presence and types of critical files/directories:
    *  - $HOME/.config/home-manager/home.nix (regular file)
    *  - $HOME/.local/share/home-manager (directory)
    *  - $HOME/.nix-profile (directory)
    *  - $HOME/.nix-defexpr (directory)
    *  - $HOME/.nix-channels (regular file)
    *  - $HOME/.local/state/nix (directory)
    *
    * Aggregates shell output and errors, returning early on any failure with
    * accumulated output and error messages.
    *
    * @return std::tuple<bool, QStringList, QStringList>
    *   - bool: success (true) if all checks pass, false otherwise
    *   - std::vector<std::string>: collected stdout/stderr output lines
    *   - std::vector<std::string>: collected error messages (includes validation failures)
    */
    std::tuple<bool, QStringList, QStringList>
    detect_nix_home_manager();
}

#endif // NIX_SETUP_H