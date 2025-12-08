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

#include "nix-setup.h"

namespace SetupNixHomeManager {
    std::tuple<bool, QStringList, QStringList>
    install_nix_home_manager(const QString& version) { // command is directly copied from here: https://gitlab.com/tuxecure/crackle-apt/crackle/-/blob/v0.5.4.1/lib/setupfunc#L86 though it is modifed.

        // Initialize the lists for output and full error
        QStringList output;
        QStringList full_output;
        QStringList full_error;
        QStringList error;
        QString command; 
        bool success;
        bool command_success;
        
        if (version != QString()) { // set command based on if we want to choose a specific release or use unstable channel for eveything.
            command = QStringLiteral(R"(/bin/bash <<'EOF'
source $HOME/.profile
check_cmd() {
    command -v "$1" > /dev/null 2>&1
}
if ! check_cmd nix; then
    sh <(curl --proto '=https' --tlsv1.2 -L https://nixos.org/nix/install) --no-daemon --no-channel-add;
    . $HOME/.nix-profile/etc/profile.d/nix.sh
    nix-channel --add https://nixos.org/channels/nixos-%1 nixpkgs
    nix-channel --update
fi
if ! check_cmd home-manager; then
    nix-channel --add https://github.com/nix-community/home-manager/archive/release-%1.tar.gz home-manager
    nix-channel --update
    nix-shell '<home-manager>' -A install
fi
EOF)").arg(version);
        } else { // modification here is hardcoded to urls that are dynamic as in, unstable channel is not specified version so likely-hood of change is low.
            command = QStringLiteral(R"(/bin/bash  <<'EOF'
source $HOME/.profile
check_cmd() {
    command -v "$1" > /dev/null 2>&1
}
if ! check_cmd nix; then
    sh <(curl --proto '=https' --tlsv1.2 -L https://nixos.org/nix/install) --no-daemon;
    . $HOME/.nix-profile/etc/profile.d/nix.sh
    nix-channel --update
fi
if ! check_cmd home-manager; then
    sudo nix-channel --add https://github.com/nix-community/home-manager/archive/master.tar.gz home-manager
    nix-channel --update
    nix-shell '<home-manager>' -A install
fi
EOF)");
        }

        // std::tie(success, output, error) = SetupCurlXz::install_curlxz(extraction_path);
        // full_error.append(error);
        // full_output.append(output);
        // if (!success) {
        //     full_error << QStringLiteral("failed to install temporary files (xz/curl).");
        //     return {false, full_output, full_error};
        // }

        std::tie(command_success, output, error) = exec_bash(command); // don't abort yet — we must clean up
        if (!command_success) {
            error << QStringLiteral("failed to install nix/home-manager.");
        }
        full_error.append(error);
        full_output.append(output);

        // std::tie(success, output, error) = SetupCurlXz::remove_curlxz(extraction_path); // cleanup
        // full_error.append(error);
        // full_output.append(output);
        // if (!success) {
        //     full_error << QStringLiteral("failed to clean temporary files!! (xz/curl).");
        // }

        return {command_success, full_output, full_error};
    }

    std::tuple<bool, QStringList, QStringList>
    uninstall_nix_home_manager() { // we are copying https://gitlab.com/tuxecure/crackle-apt/crackle/-/blob/v0.5.4.1/crackle#L161 with some modifications.
        // Initialize the lists for output and full error
        QStringList output;
        QStringList full_error;
        bool success; 
        // /nix/store has modifed perms that do not let you rm -rf it, because we are deleting /nix we don't care about the potential vulnerability that we make by chmod 777 /nix
        QString command = QStringLiteral(R"(/bin/bash <<'EOF'
chmod 777 -R /nix/*;
rm -rf /nix/*;
chmod 775 -R /nix/* || true;
rm -rf $HOME/.config/home-manager;
rm -rf $HOME/.local/share/home-manager;
rm -rf $HOME/.nix-profile $HOME/.nix-defexpr $HOME/.nix-channels $HOME/.nix-command-errors.log $HOME/.cache/nix $HOME/.local/share/nix $HOME/.local/state/nix/;
EOF)");

        std::tie(success, output, full_error) = exec_bash(command); // actually run the bash code.

        return {true, output, full_error}; // return tuple values
    }
}

// namespace SetupCurlXz {

//     std::tuple<bool, QStringList, QStringList>
//     install_curlxz(const QString& extraction_path) {
//         QStringList output;
//         QStringList full_error;
//         bool success;
//         QString detect_command = QStringLiteral(R"(/bin/bash <<'EOF'
// source $HOME/.profile
// check_cmd() {
//     command -v "$1" > /dev/null 2>&1
// }
// if check_cmd curl; then
//     if check_cmd xz; then
//         exit 0
//     else
//         exit 1
//     fi
// else
//     exit 1
// fi
// EOF)");
//         QString command = QStringLiteral(R"(/bin/bash <<'EOF'
// set -euo pipefail
// APTCACHE=${APTCACHE:-${XDG_CACHE_HOME:-$HOME/.cache}/apt}
// APTSTATE=${APTSTATE:-${XDG_STATE_HOME:-$HOME/.local/state}/apt}
// APTSTATELOCK=${APTSTATELOCK:-${XDG_STATE_HOME:-$HOME/.local/state}/dpkg}
// APTCONFIG=${APTCONFIG:-${XDG_CONFIG_HOME:-$HOME/.config}/apt}
// OUTDIR="%1"
// [[ -d "$OUTDIR" ]] || mkdir -p "$OUTDIR";
// [[ -d "$APTCACHE" ]] || mkdir -p "$APTCACHE";
// [[ -d "$APTSTATE" ]] || mkdir -p "$APTSTATE";
// [[ -d "$APTSTATELOCK" ]] || mkdir -p "$APTSTATELOCK";
// [[ -d "$APTCONFIG" ]] || {
// mkdir -p $APTCONFIG/sources.list.d $APTCONFIG/preferences.d $APTCONFIG/trusted.gpg.d;
// ln -s /etc/apt/sources.list.d/* $APTCONFIG/sources.list.d;
// cp /etc/apt/sources.list "$APTCONFIG/sources.list" || true;
// if ls /etc/apt/trusted.gpg.d >/dev/null 2>&1; then
//   ln -s /etc/apt/trusted.gpg.d/* "$APTCONFIG"/trusted.gpg.d/
// fi
// }
// mkdir -p "$APTCACHE/archives" "$APTSTATE" "$APTCONFIG" "$OUTDIR"
// APT_GET="apt-get -o Dir::Cache=$APTCACHE -o Dir::State=$APTSTATE -o Dir::Etc=$APTCONFIG"
// # Ensure lists and download curl and xz-utils .debs only
// $APT_GET update -qq
// $APT_GET install --download-only -y curl xz-utils
// # Extract /usr/bin from each downloaded .deb into OUTDIR
// for deb in "$APTCACHE"/archives/{curl*,xz-utils*}.deb; do
//   [ -f "$deb" ] || continue
//   tmpdir=$(mktemp -d)
//   dpkg-deb -x "$deb" "$tmpdir"
//   if [ -d "$tmpdir"/usr/bin ]; then
//     mkdir -p "$OUTDIR"
//     find "$tmpdir"/usr/bin -type f -exec cp --preserve=mode,timestamps {} "$OUTDIR"/ \;
//   fi
//   rm -rf "$tmpdir"
// done
// EOF)").arg(extraction_path);
//         std::tie(success, output, full_error) = exec_bash(detect_command); // if already exists in path no reason to install
//         if (success) {
//             return {success, output, full_error}; // return tuple values
//         }
//         std::tie(success, output, full_error) = exec_bash(command); // actually run the bash code.
//         return {success, output, full_error}; // return tuple values

//     }

//     std::tuple<bool, QStringList, QStringList>
//     remove_curlxz(const QString& extraction_path) {
//         QStringList output;
//         QStringList full_error;
//         bool success; 
//         QString command = QStringLiteral(R"(/bin/bash <<'EOF'
// set -euo pipefail
// APTCACHE=${APTCACHE:-${XDG_CACHE_HOME:-$HOME/.cache}/apt}
// APTSTATE=${APTSTATE:-${XDG_STATE_HOME:-$HOME/.local/state}/apt}
// APTSTATELOCK=${APTSTATELOCK:-${XDG_STATE_HOME:-$HOME/.local/state}/dpkg}
// APTCONFIG=${APTCONFIG:-${XDG_CONFIG_HOME:-$HOME/.config}/apt}
// OUTDIR="%1"
// rm -rf $OUTDIR $APTCONFIG $APTSTATE $APTSTATELOCK $APTCACHE;
// EOF)").arg(extraction_path);
//         std::tie(success, output, full_error) = exec_bash(command); // actually run the bash code.
//         return {success, output, full_error}; // return tuple values

//     }
// }

namespace DetectNixHomeManager{
    std::tuple<bool, QStringList, QStringList>
    detect_nix_home_manager() {
        QStringList full_output;
        QStringList output;
        QStringList full_error;
        QStringList error;
        bool success = true;
        QString command = QStringLiteral(R"(/bin/bash <<'EOF'
source $HOME/.profile
check_cmd() {
    command -v "$1" > /dev/null 2>&1
}
if check_cmd home-manager; then
    exit 0
else
    exit 1
fi
EOF)");
        std::tie(success, output, error) = exec_bash(command); // we don't care about output here
        full_error.append(error);
        full_output.append(output);
        if (!success) {
            return {false, full_output, full_error};
        }

        std::tie(success, output, error) = exec_bash(QStringLiteral("echo $HOME"));
        full_error.append(error);
        full_output.append(output);
        if (!success) {
            return {false, full_output, full_error};
        }

        // Join output lines into a single QString (home directory)
        QString output_string = output.join(QString());

        // Proceed to directory/file checks to validate integrity.
        // Use QFileInfo and QDir (Qt) instead of std::filesystem for consistency.
        #include <QFileInfo>
        #include <QDir>

        QFileInfo fi_home_manager(output_string + "/.config/home-manager/home.nix");
        if (!fi_home_manager.isFile()) {
            full_error << QStringLiteral("(%1) is invalid or not a regular file").arg(fi_home_manager.filePath());
            return {false, full_output, full_error};
        }

        QDir d_nix_profile(output_string + "/.nix-profile");
        if (!d_nix_profile.exists() || !d_nix_profile.isReadable()) {
            full_error << QStringLiteral("(%1) is invalid or not a directory").arg(d_nix_profile.absolutePath());
            return {false, full_output, full_error};
        }

        QDir d_nix_defexpr(output_string + "/.nix-defexpr");
        if (!d_nix_defexpr.exists() || !d_nix_defexpr.isReadable()) {
            full_error << QStringLiteral("(%1) is invalid or not a directory").arg(d_nix_defexpr.absolutePath());
            return {false, full_output, full_error};
        }

        QFileInfo fi_nix_channels(output_string + "/.nix-channels");
        if (!fi_nix_channels.isFile()) {
            full_error << QStringLiteral("(%1) is invalid or not a regular file").arg(fi_nix_channels.filePath());
            return {false, full_output, full_error};
        }

        QDir d_local_state_nix(output_string + "/.local/state/nix");
        if (!d_local_state_nix.exists() || !d_local_state_nix.isReadable()) {
            full_error << QStringLiteral("(%1) is invalid or not a directory").arg(d_local_state_nix.absolutePath());
            return {false, full_output, full_error};
        }

        full_output << QStringLiteral("Successfully validated installation!");
        return {true, full_output, full_error};
    }
}