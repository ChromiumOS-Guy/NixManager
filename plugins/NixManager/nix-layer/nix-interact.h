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

#ifndef NIX_INTERACT_H
#define NIX_INTERACT_H

#include "../libs/openprocess.h"  // Include openProcess function declaration

#include <tuple> // Required for std::tuple return type
#include <QString>
#include <QRegularExpression>
#include <QStringList>

// Forward declaration for openProcess if openprocess.h is not included directly here.
// However, since openprocess.h is included in the .cpp, and its types are standard,
// we just need to ensure std::vector is available.
// #include "openprocess.h" // Generally, header should not include .cpp implementation headers unless absolutely necessary for interface types

namespace HomeManager {
    /**
    * @brief Applies a NixOS configuration by running 'home-manager build' and parses its output.
    *
    * This function executes the 'home-manager build' command. It captures both standard
    * output and standard error. It specifically filters the full error output to
    * identify lines starting with "error: attribute" and extracts the specific error
    * message following this prefix, adding each as an individual entry to 'simple_error'.
    *
    * @return A tuple containing three lists of strings:
    * - bool detected: True if success (command exited 0),
    * - sQStringList output: Lines from the standard output of the command.
    * - QStringList simple_error: Specific error messages related to
    * "attribute" issues, with each message as a separate item.
    * - QStringList full_error: All lines from the standard error output
    * of the command.
    */
    std::tuple<bool, QStringList, QStringList, QStringList>
    // hm_switch(const bool allow_insecure = false);
    hm_switch(const bool allow_insecure);

    /**
    * @brief Detects version of home-manager.
    *
    * This function attempts to get the version of the currently installed home-manager
    *
    * @return A tuple containing:
    * - bool detected: True if success (command exited 0),
    * false otherwise (stderr output or command execution failure).
    * - QStringList output: Lines from the standard output of the command.
    * - QStringList full_error: All lines from the standard error output
    */
    std::tuple<bool, QStringList, QStringList>
    hm_version();

    /**
    * @brief Deletes all non-current generations according to timestamp.
    *
    * This function attempts to expire (delete) home-manager genrations older then date N.
    *
    * @return A tuple containing:
    * - bool detected: True if success (command exited 0),
    * - QStringList output: Lines from the standard output of the command.
    * - QStringList full_error: All lines from the standard error output
    */
    std::tuple<bool, QStringList, QStringList>
    // hm_expire_generations(const std::string& timestamp = "-30 days");
    hm_expire_generations(const QString& timestamp); 

    /**
    * @brief Queries the home-manager for available generations.
    *
    * This function constructs and executes a home-manager command to list all available
    * generations of the current home-manager environment. It processes the output to extract
    * relevant generation IDs and their timestamps, returning them in a structured
    * format. If the command fails or no generations are found, appropriate error
    * messages are returned.
    *
    * @return A tuple containing:
    * - bool detected: True if success (command exited 0),
    * - QStringList generations: A vector of strings formatted as JSON-like
    * objects containing generation IDs, timestamps, and their paths, or error
    * messages if the query failed.
    * - QStringList error: All lines from the standard error output
    *   of the command, including any general execution errors or messages.
    */
    std::tuple<bool, QStringList, QStringList>
    hm_list_generations();
}

namespace NixEnv {
    /**
    * @brief Queries the Nix package manager for available packages based on a search term.
    *
    * This function constructs and executes a Nix command to list all available packages
    * that match the specified search term. It processes the output to extract relevant
    * package names and their summaries, returning them in a structured format. If the command
    * fails or no packages are found, appropriate error messages are returned.
    *
    * @param quarry A string representing the search term for the Nix command (e.g., "firefox").
    * @return A tuple containing:
    * - bool detected: True if success (command exited 0),
    * false otherwise (e.g., no packages found, command timeout, or execution error).
    * - QStringList packages: A vector of strings formatted as JSON-like
    * objects containing package names and summaries. 
    * - QStringList full_error: error messages if the query failed.
    */
    std::tuple<bool, QStringList, QStringList>
    quarry(const QString& quarry);

    /**
    * @brief Queries the Nix package manager for available generations.
    *
    * This function constructs and executes a Nix command to list all available
    * generations of the current Nix environment. It processes the output to extract
    * relevant generation IDs and their timestamps, returning them in a structured
    * format. If the command fails or no generations are found, appropriate error
    * messages are returned.
    *
    * @return A tuple containing:
    * - bool detected: True if success (command exited 0),
    * - QStringList generations: A vector of strings formatted as JSON-like
    * objects containing generation IDs, timestamps, and their current status, or error
    * messages if the query failed.
    * - QStringList> error: All lines from the standard error output
    *   of the command, including any general execution errors or messages.
    */
    std::tuple<bool, QStringList, QStringList>
    list_generations();

    /**
    * @brief Switch to a specified generation in the Nix environment.
    *
    * This function executes the 'nix-env --switch-generation' command to switch
    * the specified generation identified by the provided generation ID. It captures
    * both standard output and standard error from the command execution. Any errors
    * encountered during the process, including failures to execute the command,
    * are captured in the error vector.
    *
    * @param generation_id A string representing the ID of the generation to be switched to.
    *
    * @return A tuple containing two lists of strings:
    * - bool detected: True if success (command exited 0),
    * - QStringList output: Lines from the standard output of the command.
    * - QStringList error: All lines from the standard error output of the command,
    * including any general execution errors.
    */
    std::tuple<bool, QStringList, QStringList>
    switch_generation(const QString& generation_id);

    /**
    * @brief Deletes a specified generation in the Nix environment.
    *
    * This function executes the 'nix-env --delete-generations' command to remove
    * the specified generation identified by the provided generation ID. It captures
    * both standard output and standard error from the command execution. Any errors
    * encountered during the process, including failures to execute the command,
    * are captured in the error vector.
    *
    * @param generation_id A string representing the ID of the generation to be deleted.
    *
    * @return A tuple containing two lists of strings:
    * - bool detected: True if success (command exited 0),
    * - QStringList output: Lines from the standard output of the command.
    * - QStringListerror: All lines from the standard error output of the command,
    * including any general execution errors.
    */
    std::tuple<bool, QStringList, QStringList>
    delete_generation(const QString& generation_id);

    /**
    * @brief Deletes all old generations in the Nix environment.
    *
    * This function executes the 'nix-env --delete-generations old' command to remove
    * all generations that are older than the current one. It captures both standard
    * output and standard error from the command execution. Any errors encountered
    * during the process, including failures to execute the command, are captured
    * in the error vector.
    *
    * @return A tuple containing two lists of strings:
    * - bool detected: True if success (command exited 0),
    * - QStringList output: Lines from the standard output of the command.
    * - QStringList error: All lines from the standard error output of the command,
    * including any general execution errors.
    */
    std::tuple<bool, QStringList, QStringList>
    delete_old_generations();

}

namespace NixChannel {
    /**
    * @brief Updates Nix channels by running 'nix-channel --update'.
    *
    * This function executes the 'nix-channel --update' command to refresh the
    * Nix channels (e.g., nixpkgs, home-manager) to their latest versions.
    * It captures both standard output and standard error from the command execution.
    * Any errors encountered during the process, including failures to execute the
    * command, are captured in the error vector.
    *
    * @return A tuple containing two lists of strings:
    * - bool detected: True if success (command exited 0),
    * - QStringList output: Lines from the standard output of the command.
    * - QStringList error: All lines from the standard error output of the command,
    * including any general execution errors.
    */
    std::tuple<bool, QStringList, QStringList>
    update_channels();

    /**
    * @brief Lists the Nix channels by running 'nix-channel --list'.
    *
    * This function executes the 'nix-channel --list' command to retrieve the
    * currently configured Nix channels. It captures both standard output and
    * standard error from the command execution. If no channels are found, an
    * appropriate message is added to the error vector. The output is processed
    * into a structured format, representing each channel as a JSON-like string
    * containing its name and URL.
    *
    * @return A tuple containing two lists of strings:
    * - bool detected: True if success (command exited 0),
    * - QStringList processed_output: JSON-like strings representing
    *   each channel's name and URL.
    * - QStringList error: All lines from the standard error output
    *   of the command, including any general execution errors or messages.
    */
    std::tuple<bool, QStringList, QStringList> 
    list_channels();

    /**
    * @brief Adds a new Nix channel by running 'nix-channel --add'.
    *
    * This function executes the 'nix-channel --add' command to add a new
    * channel specified by the provided URL and name. It captures both
    * standard output and standard error from the command execution. Any
    * errors encountered during the process, including failures to execute
    * the command, are captured in the error vector.
    *
    * @param url The URL of the channel to be added.
    * @param name The name to assign to the new channel.
    *
    * @return A tuple containing two lists of strings:
    * - bool detected: True if success (command exited 0),
    * - QStringList output: Lines from the standard output of the command.
    * - QStringList error: All lines from the standard error output of the command,
    * including any general execution errors.
    */
    std::tuple<bool, QStringList, QStringList> 
    add_channel(const QString& url, const QString& name);

    /**
    * @brief Removes an existing Nix channel by running 'nix-channel --remove'.
    *
    * This function executes the 'nix-channel --remove' command to remove
    * a channel specified by the provided name. It captures both standard
    * output and standard error from the command execution. Any errors
    * encountered during the process, including failures to execute the
    * command, are captured in the error vector.
    *
    * @param name The name of the channel to be removed.
    *
    * @return A tuple containing two lists of strings:
    * - bool detected: True if success (command exited 0),
    * - QStringList output: Lines from the standard output of the command.
    * - QStringList error: All lines from the standard error output of the command,
    * including any general execution errors.
    */
    std::tuple<bool, QStringList, QStringList> 
    remove_channel(const QString& name);
}

#endif // NIX_INTERACT_H