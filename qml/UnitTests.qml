/*
 * This file is part of system-settings
 *
 * Copyright (C) 2013 Canonical Ltd.
 *
 * Contact: Alberto Mardegan <alberto.mardegan@canonical.com>
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

import QtQuick 2.7
import Lomiri.Components 1.3
//import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0

import NixManagerPlugin 1.0




MainView {
    id: root
    objectName: "NixManagerPluginUnitTests"
    property var item: null
    
    property string currentRequestId: ""

    Connections {
        target: NixManagerPlugin
        
        // This handler fires for *all* completed operations
        onOperation_result: (resultJson, receivedId) => {
            
            // 3. Match the ID to ensure it's the result we are waiting for
            if (receivedId === root.currentRequestId) {
                console.log("Received result for requested ID:", receivedId);
                
                // --- Process the resultJson string here ---
                try {
                    const result = JSON.parse(resultJson);
                    var result;
                    if (result.success === true) {
                        console.log(functionName + " SUCCEEDED.");
                        console.log("Result:", JSON.stringify(result, null, 2));
                    } else {
                        console.error(functionName + " FAILED.");
                        console.error("Result:", JSON.stringify(result, null, 2));
                    }
                    console.log("--- Finished Test: " + functionName + " (" + (result.success ? "SUCCEEDED" : "FAILED") + ") ---");
                } catch(e) {
                    console.error(functionName + " FAILED: JSON parse error.");
                    console.error("Input:", resultJsonString, "Error:", e.toString());
                    console.log("--- Finished Test: " + functionName + " (FAILED) ---");
                }
                
                // Reset the ID after processing
                root.currentRequestId = ""; 
            }
        }
    }

    // Utility function to execute a test, parse the result, and check for success
    function runTest(functionName, qmlFunctionCall) {
        console.log("--- Starting Test: " + functionName + " ---");
        var resultJsonString = qmlFunctionCall();
        var result;
        try {
            result = JSON.parse(resultJsonString);
        } catch (e) {
            console.error(functionName + " FAILED: JSON parse error.");
            console.error("Input:", resultJsonString, "Error:", e.toString());
            console.log("--- Finished Test: " + functionName + " (FAILED) ---");
            return;
        }

        if (result.success === true) {
            console.log(functionName + " SUCCEEDED.");
            console.log("Result:", JSON.stringify(result, null, 2));
        } else {
            console.error(functionName + " FAILED.");
            console.error("Result:", JSON.stringify(result, null, 2));
        }
        console.log("--- Finished Test: " + functionName + " (" + (result.success ? "SUCCEEDED" : "FAILED") + ") ---");
    }

    // --- Unit Tests for NixManager Functions ---

    Component.onCompleted: {
        // // --- Test 1: hm_switch() (Default: no allow_insecure) ---
        // runTest("hm_switch (default)", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_hm_switch(root.currentRequestId);
        // });

        // // --- Test 2: hm_switch(true) (Allow insecure) ---
        // runTest("hm_switch (allow_insecure)", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_hm_switch(root.currentRequestId, true);
        // });

        // // --- Test 3: hm_version() ---
        // runTest("hm_version", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_hm_version(root.currentRequestId);
        // });

        // // --- Test 4: search_packages(quarry, local=false) (API Search) ---
        // runTest("search_packages (API)", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_search_packages(root.currentRequestId, "firefox", false, "https://search.devbox.sh");
        // }, function(result) {
        //     // Specific action for search_packages: Log the output
        //     console.log("Search Output (API - First 5 items):\n", JSON.stringify(result.output.slice(0, 5), null, 2));
        // });

        // // --- Test 5: search_packages(quarry, local=true) (Local Search) ---
        // runTest("search_packages (Local)", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_search_packages(currentRequestId, "firefox", true);
        // }, function(result) {
        //     // Specific action for search_packages: Log the output
        //     console.log("Search Output (Local - First 5 items):\n", JSON.stringify(result.output.slice(0, 5), null, 2));
        // });

        // // --- Test 6: update_channels() ---
        // runTest("update_channels", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_update_channels(currentRequestId);
        // });

        // // --- Test 7: list_channels() ---
        // runTest("list_channels", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_list_channels(currentRequestId);
        // }, function(result) {
        //     // Check if the output is an array (list of dictionaries)
        //     if (Array.isArray(result.output) && result.output.length > 0) {
        //         console.log("list_channels: Found " + result.output.length + " channels. Example channel:", JSON.stringify(result.output[0]));
        //     } else {
        //         console.log("list_channels: Channels list is empty or not an array.");
        //     }
        // });

        // // --- Test 8: add_channel() ---
        // // NOTE: Use a temporary/safe channel name/URL. This relies on previous state being clean.
        // runTest("add_channel (testchannel)", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_add_channel(root.currentRequestId, "https://nixos.org/channels/nixos-23.11", "nixpkgs-2311-test");
        // });

        // // --- Test 9: remove_channel() ---
        // // Attempt to remove the channel added in the previous test.
        // runTest("remove_channel (testchannel)", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_remove_channel(root.currentRequestId, "nixpkgs-2311-test");
        // });

        // // --- Test 10: list_generations() ---
        // runTest("list_generations", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_list_generations(root.currentRequestId);
        // }, function(result) {
        //     // Check if the output is an array (list of dictionaries)
        //     if (Array.isArray(result.output) && result.output.length > 0) {
        //         console.log("list_generations: Found " + result.output.length + " generations. Example generation:", JSON.stringify(result.output[0]));
        //     } else {
        //         console.log("list_generations: Generations list is empty or not an array.");
        //     }
        // });

        // // --- Test 11: switch_generation() ---
        // // WARNING: Highly system state dependent. Using '1' as a common initial ID.
        // runTest("switch_generation (4)", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_switch_generation(root.currentRequestId, "4"); // specific to my machine i switch to gen 1 so i can switch back to gen 4 so delete_old_generations test will work well.
        // });

        // // --- Test 12: delete_generation() ---
        // // WARNING: Highly system state dependent. Using '2' as a placeholder.
        // runTest("delete_generation (2)", function() { 
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_delete_generation(root.currentRequestId, "2"); // again specific to my machine please change.
        // });

        // // --- Test 13: delete_old_generations() ---
        // runTest("delete_old_generations", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_delete_old_generations(root.currentRequestId);
        // });

        // // --- Test 14: hm_expire_generations() (Default: -30 days) ---
        // runTest("hm_expire_generations (default)", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_hm_expire_generations(root.currentRequestId);
        // });

        // // --- Test 15: hm_list_generations() ---
        // runTest("hm_list_generations", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_hm_list_generations(root.currentRequestId);
        // }, function(result) {
        //     // Check if the output is an array (list of dictionaries)
        //     if (Array.isArray(result.output) && result.output.length > 0) {
        //         console.log("hm_list_generations: Found " + result.output.length + " HM generations. Example generation:", JSON.stringify(result.output[0]));
        //     } else {
        //         console.log("hm_list_generations: HM Generations list is empty or not an array.");
        //     }
        // });

        // // --- Test 17: read_packages ---
        // runTest("read_packages", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_read_packages(root.currentRequestId);
        // });

        // // --- Test 18: add_packages (Placeholder Test) ---
        // // Checks function signature and basic success/fail on a hypothetical package.
        // runTest("add_packages (pkgs.firefox)", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_add_packages(root.currentRequestId, '["pkgs.firefox"]');
        // });

        // // --- Test 19: delete_packages (Placeholder Test) ---
        // // Checks function signature and basic success/fail on a hypothetical package.
        // runTest("delete_packages (pkgs.firefox)", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_delete_packages(root.currentRequestId, '["pkgs.firefox"]');
        // });

        // // --- Test 20: install nix/home-manager  ---
        // runTest("install nix/home-manager 25.05 stable", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_install_nix_home_manager(root.currentRequestId, "25.05");
        // });

        // // --- Test 21: detect nix/home-manager ---
        // runTest("detect nix/home-manager", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_detect_nix_home_manager(root.currentRequestId);
        // });

        // // --- Test 22: uninstall nix/home-manager ---
        // runTest("uninstall nix/home-manager", function() {
        //     root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        //     return NixManagerPlugin.request_uninstall_nix_home_manager(root.currentRequestId);
        // });
        
    }

    // Retained the minimal layout structure from your original example
    ColumnLayout {
        anchors.fill: parent
        spacing: units.gu(1)

        // No longer relying on a general jsonDataObject for display, as tests run on completion.
        // You can add simple labels here to show the page loaded.
        Label {
            text: "Nix Manager Unit Tests Loaded."
            font.pixelSize: units.gu(2.5)
            color: Lomiri.Components.Theme.palette.normal.text
        }
        Label {
            text: "Check the console output for test results."
            font.pixelSize: units.gu(2)
            color: Lomiri.Components.Theme.palette.normal.text
        }
    }
}