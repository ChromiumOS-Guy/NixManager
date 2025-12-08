/*
 * Copyright (C) 2025  ChromiumOS-Guy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * nixmanager is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.7

import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0

import Lomiri.Components 1.3
import Lomiri.Components.Popups 1.3
import NixManagerPlugin 1.0

Page {
    id: advancedsettingsPage
    property bool timestamp_action: false
    property bool uninstall_action: false
    property bool search_settings: false
    property bool do_insecure_packages: false

    Component {
        id: dialog
        Dialog {
            id: dialogue
            title: "placeholder"
            text: "placeholder"

            property string timestamp: ""
            property string timeout: ""
            property string url: ""
            property bool local_search: false

            Component.onCompleted: {
                if (uninstall_action == true) {
                    this.title = "Uninstall nix/home-manager?";
                    this.text = "This will uninstall everything, including Nix and Home Manager.";
                } else if (do_insecure_packages == true) {
                    this.title = "Allow insecure packages?";
                    this.text = "This will allow insecure packages (packages with CVE vulnerabilities). \n\nThis action is irreversible — to restore the previous state you must reinstall Nix which will disable this setting.";
                } else if (timestamp_action == true) {
                    this.title = "Change timestamp?";
                    this.text = "This will change the timestamp used to expire generations.";
                } else if (search_settings == true) {
                    this.title = "Search settings.";
                    this.text = "Search settings, only use if you know what these terms mean. \n Fair Warning local search is extremly resource intesive and may even crash on weaker phones like OnePlus Nord N100.";
                }
            }

            // API URL row
            RowLayout {
                visible: (search_settings && local_search == false)
                enabled: (search_settings && local_search == false)
                spacing: units.gu(1)
                Label {
                    text: i18n.tr("API url:")
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredWidth: parent.width * 0.35
                    wrapMode: Text.WordWrap
                }
                TextField {
                    text: url
                    Layout.fillWidth: true
                    property string lastValid: ""
                    // allow any characters except whitespace
                    readonly property var relaxRewhitespace: /^[^\s]*$/
                    // basic but practical URL regex (http/https, optional port, path/query/fragment)
                    // - requires protocol (http or https)
                    // - domain (with labels and TLD) or IPv4
                    // - optional :port
                    // - optional path/query/fragment
                    readonly property var relaxRe: /^(https?:\/\/)(?:(([A-Za-z0-9-]{1,63}\.)+[A-Za-z]{2,63})|(\d{1,3}(?:\.\d{1,3}){3}))(?:\:\d{1,5})?(?:[\/?#][^\s]*)?$/
                    onTextChanged: {
                        if (relaxRewhitespace.test(text)) {
                            lastValid = text;
                        } else {
                            // revert to previous valid value if the new text contains whitespace
                            text = lastValid;
                        }
                    }
                    onAccepted: {
                        if (relaxRe.test(text)) {
                            url = text.replace(/\s+/g, "") // final sanitize just in case
                        }
                    }
                }
            }

            // Timestamp/Timeout row
            RowLayout {
                visible: timestamp_action || (search_settings && local_search == false)
                enabled: timestamp_action || (search_settings && local_search == false)
                spacing: units.gu(1)
                Label {
                    text: timestamp_action == true ? i18n.tr("New timestamp (in days):") : i18n.tr("New timeout (in seconds):")
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredWidth: parent.width * 0.35
                    wrapMode: Text.WordWrap
                }
                TextField {
                    text: timestamp
                    Layout.fillWidth: true
                    property string lastValid: ""
                    // only digits 0-9, allow empty string while typing
                    readonly property var relaxRe: /^[0-9]*$/

                    onTextChanged: {
                        if (relaxRe.test(text)) {
                            lastValid = text;
                            if (timestamp_action == true) {timestamp = text.replace(/\D+/g, "");}
                            if (search_settings == true) {timeout = text.replace(/\D+/g, "");}
                            
                        } else {
                            text = lastValid
                        }
                    }

                    onAccepted: {
                        if (relaxRe.test(text)) {
                            // final sanitize: remove non-digits
                            if (timestamp_action == true) {timestamp = text.replace(/\D+/g, "");}
                            if (search_settings == true) {timeout = text.replace(/\D+/g, "");}
                        } 
                    }
                }
            }

            // local search row
            RowLayout {
                visible: search_settings
                enabled: search_settings
                spacing: units.gu(1)
                Label {
                    text: i18n.tr("Use local search:")
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredWidth: parent.width * 0.35
                    wrapMode: Text.WordWrap
                }
                CheckBox {
                    checked: local_search
                    onCheckedChanged: local_search = checked
                }
            }

            Button {
                text: timestamp_action == true ? "Change" : uninstall_action == true ? "Uninstall" : do_insecure_packages == true ? "Allow" : search_settings == true ? "Apply" : "Unknown"
                color: uninstall_action == false ? theme.palette.normal.positive : theme.palette.normal.negative
                enabled: (timestamp != "" && timestamp_action == true) || do_insecure_packages == true || uninstall_action == true || search_settings == true
                onClicked: {
                    if (do_insecure_packages == true) {
                        root.allow_insecure_pakcages = true;
                    } else if (timestamp_action == true) {
                        root.expire_generation_timestamp = "-" + timestamp + " days"
                    } else if (uninstall_action == true) {
                        root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                        NixManagerPlugin.request_uninstall_nix_home_manager(root.currentRequestId);
                        advancedsettingsList.visible = false;
                        advancedsettingsList.enabled = false;
                        loadingbar.visible = true;
                        loadingbar.enabled = true;
                    } else if (search_settings == true) {
                        if (timeout != "") {root.api_timeout = timeout;}
                        if (url != "") {root.search_api_url =  url;}
                        if (local_search != "") {root.enable_local_search = local_search;}
                    }
                    PopupUtils.close(dialogue)
                }
            }

            Button {
                text: "Reset"
                enabled: timestamp_action == true || (search_settings == true && (enable_local_search != false || search_api_url != "https://search.devbox.sh" || api_timeout != 10))
                visible: (timestamp_action == true && root.expire_generation_timestamp != "-30 days") || (search_settings == true && (enable_local_search != false || search_api_url != "https://search.devbox.sh" || api_timeout != 10))
                onClicked: {
                    if (timestamp_action == true) {root.expire_generation_timestamp = "-30 days";}
                    if (search_settings == true) {
                        root.popPage()
                    }
                    PopupUtils.close(dialogue)
                }
            }

            Button {
                text: "cancel"
                onClicked: PopupUtils.close(dialogue)
            }
        }
    }

    Connections {
        target: NixManagerPlugin
        
        // This handler fires for *all* completed operations
        onOperation_result: (resultJson, receivedId, operation) => {
            
            // 3. Match the ID to ensure it's the result we are waiting for
            if (receivedId === receivedId) {
                console.log("Received result for requested ID:", receivedId);
                
                // --- Process the resultJson string here ---
                try {
                    const result = JSON.parse(resultJson);
                    // Example processing:
                    if (result.success) {
                        if (operation == "uninstall_nix_home_manager") {
                            root.popPage();
                            root.reset_settings();
                            root.init_main();
                        }
                        
                    } else {
                        advancedsettingsList.visible = false;
                        advancedsettingsList.enabled = false;
                        showerror.visible = true;
                        showerror.enabled = true;
                        simpleerror.chosen = result.simple_error.join(' ');
                        showerror.error = result.full_error.join(' ');
                    }
                } catch(e) {
                    console.error("Failed to parse result JSON:", e);
                }
                
                // Reset the ID after processing
                root.currentRequestId = ""; 
            }
        }
    }

    header: PageHeader {
        id: header
        title: i18n.tr('Advanced settings')

        leadingActionBar.actions: [
            Action {
                text: i18n.tr('Back')
                iconName: 'toolkit_chevron-rtl_1gu'
                onTriggered: root.popPage()
            }
        ]
    }

    Flickable {
        anchors {
            topMargin: units.gu(2)
            bottomMargin: units.gu(2)
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        visible: advancedsettingsList.visible == true
        enabled: advancedsettingsList.enabled == true

        contentWidth: parent.width
        contentHeight: advancedsettingsList.implicitHeight
        clip: true
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        interactive: true

        ColumnLayout {
            id: advancedsettingsList
            spacing: units.gu(2)
            enabled: true
            visible: true
            // anchors {
            //     // topMargin: units.gu(2)
            //     // bottomMargin: units.gu(2)
            //     top: parent.top
            //     left: parent.left
            //     right: parent.right
            //     bottom: parent.bottom
            // }
            width: parent.width

            Label {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                text: i18n.tr('Adjusts the timestamp that determines when old home-manager generations expire. \nThe default of one month is sufficient for most users; change this only if you have a specific requirement (for example, longer retention for debugging or shorter retention to conserve disk space).')
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n.tr("Change timestamp")
                onClicked: {
                    uninstall_action = false;
                    timestamp_action = true;
                    search_settings = false;
                    do_insecure_packages = false;
                    PopupUtils.open(dialog);
                } // open popup to confirm apply.
            }

            Item {
                Layout.fillHeight: true
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                text: i18n.tr('Modifying search settings is not recommended unless you understand the implications. \nIncorrect changes can break search functionality or cause unexpected behavior. \nOnly adjust these settings if you are familiar with the relevant APIs and confident in troubleshooting any issues that arise.')
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                text: i18n.tr('Current values are: \n enable_local_search = ' + (root.enable_local_search == true ? "true" : " false") + "\n api_timeout (s) = " + String(root.api_timeout) + "\n search_api_url = " + root.search_api_url)
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                text: i18n.tr("Open search settings")
                onClicked: {
                    uninstall_action = false; 
                    timestamp_action = false;
                    search_settings = true;
                    do_insecure_packages = false;
                    PopupUtils.open(dialog);
                } // open popup to confirm apply.
            }

            Item {
                Layout.fillHeight: true
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                text: i18n.tr('Allowing insecure packages is strongly discouraged. \nThis may expose your system to known vulnerabilities, malware, or data loss. \nIf you enable it, packages with unresolved CVEs or weakened integrity checks can be installed. \nThis change is difficult to reverse — to fully restore safety you will need to reinstall Nix and Home Manager which will re-disable this option. \nProceed only if you understand and accept the security risks.')
            }

            Button {
                enabled: root.allow_insecure_pakcages == false
                color: theme.palette.normal.positive
                Layout.alignment: Qt.AlignHCenter
                text: i18n.tr("Allow insecure packages")
                onClicked: {
                    uninstall_action = false;
                    timestamp_action = false;
                    search_settings = false;
                    do_insecure_packages = true;
                    PopupUtils.open(dialog);
                } // open popup to confirm apply.
            }

            Item {
                Layout.fillHeight: true
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                text: i18n.tr('This will uninstall Nix and Home Manager, removing all packages and configurations installed through them. \nUser data and system-wide files outside Nix may not be affected, but any software managed by Nix/Home Manager will be deleted. \nEnsure you have backups of important nix/home-manager managed data before proceeding.')
            }

            Button {
                color: theme.palette.normal.negative
                Layout.alignment: Qt.AlignHCenter
                text: i18n.tr("Uninstall nix/home-manager")
                onClicked: {
                    uninstall_action = true;
                    timestamp_action = false;
                    search_settings = false;
                    do_insecure_packages = false;
                    PopupUtils.open(dialog);
                } // open popup to confirm apply.
            }

        }
    }

    ColumnLayout {
        id: loadingbar
        spacing: units.gu(2)
        enabled: false
        visible: false
        anchors {
            margins: units.gu(2)
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        Item {
            Layout.fillHeight: true
        }

        Label {
            id: loadinglabel
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.9
            text: i18n.tr('Uninstalling nix/home-manager, please wait.')
        }

        ProgressBar {
            Layout.alignment: Qt.AlignHCenter
            indeterminate: true
        }

        Item {
            Layout.fillHeight: true
        }

    }

    ColumnLayout {
        id: showerror
        spacing: units.gu(2)
        enabled: false
        visible: false
        anchors {
            margins: units.gu(2)
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        property string error: ""

        Label {
            id: simpleerror
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.9
            property string chosen: ""
            text: chosen
            visible: chosen != ""
        }

        Rectangle {
            color: theme.palette.normal.background
            width: parent.width
            Layout.fillHeight: true

            // 1. Use Flickable to provide the scrolling mechanism
            Flickable {
                id: flickArea
                anchors.fill: parent
                anchors.margins: 10
                clip: true // Important: Prevents text from drawing outside the Flickable bounds
                contentWidth: textContent.width
                contentHeight: textContent.height

                // 2. Place the Text element inside
                Text {
                    id: textContent
                    color: theme.palette.normal.baseText
                    // Set a fixed width (like the Flickable's width) so text wraps
                    width: flickArea.width
                    wrapMode: Text.Wrap

                    // Set the read-only text content
                    text: showerror.error
                }
            }
        }
        
        Button {
            id: copyButton
            Layout.alignment: Qt.AlignHCenter
            text: i18n.tr("Copy to Clipboard")

            onClicked: {
                Clipboard.push(showerror.error)
                this.text = i18n.tr("Copied!")
                resetTimer.start()
            }
        }

        Timer {
            id: resetTimer
            interval: 1500 // 500 milliseconds (half a second)
            repeat: false
            running: false // Starts inactive

            onTriggered: {
                // This code runs after the 500ms delay
                copyButton.text = i18n.tr("Copy to Clipboard")
            }
        }
    }
}