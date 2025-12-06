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
import NixManagerPlugin 1.0

Page {
    id: setupPage

    function handleinstallrequest(version) {
    
        optionfield.visible = false; // hide UI
        optionfield.enabled = false;

        root.currentRequestId = "VERSION_REQUEST_" + Date.now();
        NixManagerPlugin.request_install_nix_home_manager(root.currentRequestId, version); // start install

        loadingbar.visible = true; // start loading animation
        loadingbar.enabled = true;

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
                        if (operation == "install_nix_home_manager") {
                            loadingbar.visible = false; // stop loading animation
                            loadingbar.enabled = false;
                            label0.text = i18n.tr('Installation successful!');
                            label0.color = theme.palette.normal.positive;
                            reportbtn.text = i18n.tr('Continue');
                            reportbtn.clicked.connect(function() { root.popPage();})
                            report.visible = true;
                            report.enabled = true;
                        }
                        
                    } else {
                        if (operation == "install_nix_home_manager") {
                            loadingbar.visible = false; // stop loading animation
                            loadingbar.enabled = false;
                            label0.text = i18n.tr('Installation failed!');
                            label0.color = theme.palette.normal.negative;
                            reportbtn.text = i18n.tr('Details');
                            reportbtn.clicked.connect(function() {report.visible = false; report.enabled = false; showerror.visible = true; showerror.enabled = true; showerror.error = result.full_error.join(' ');})
                            report.visible = true;
                            report.enabled = true;
                        }
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
        title: i18n.tr('Nix Manager')
    }

    ColumnLayout {
        spacing: units.gu(2)
        enabled: true
        visible: true
        anchors {
            margins: units.gu(2)
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.9
            text: i18n.tr('Welcome to Nix Manager!')
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.9
            text: i18n.tr('Nix home-manager needs to be installed before you can start using this application.')
        }

        Item {
            Layout.fillHeight: true
        }

        Button {
            Layout.alignment: Qt.AlignHCenter
            text: i18n.tr("Continue")
            onClicked: {
                parent.visible = false;
                parent.enabled = false;
                optionfield.visible = true;
                optionfield.enabled = true;
            }
        }

    }

    ColumnLayout {
        id: optionfield
        spacing: units.gu(2)
        enabled: false
        visible: false
        anchors {
            topMargin: units.gu(2)
            bottomMargin: units.gu(2)
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        property string selectedValue: "25.05"
        property string selectedLabel: "25.05"

        Label {
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.9
            text: i18n.tr('Please Choose a Channel:')
        }

        ListModel {
            id: optionsModel
            ListElement { label: "25.05"; value: "25.05" }
            ListElement { label: "Unstable"; value: "" }
            ListElement { label: "Other";    value: "other" }
        }

        ListView {
            id: optionsView
            model: optionsModel
            width: parent.width
            height: optionsModel.count * units.gu(6)
            clip: true
            interactive: false

            delegate: ListItem {                      // use Item if ListItem unavailable
                width: optionsView.width
                height: units.gu(6)
                color: model.label == optionfield.selectedLabel ? theme.palette.normal.foreground : theme.palette.normal.background

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: units.gu(1)
                    text: label
                    color: theme.palette.normal.baseText
                }

                MouseArea { anchors.fill: parent
                    onClicked: {
                        if (optionfield) {
                            optionfield.selectedValue = value
                            optionfield.selectedLabel = label
                        }
                    }
                }
            }
        }



        Item {
            Layout.fillHeight: true
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.9
            text: i18n.tr("Stable, well tested, good for beginners and experienced users alike.")
            visible: optionfield.selectedLabel === "25.05"
            enabled: visible
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.9
            text: i18n.tr("Unstable, bleeding edge, riddled with bugs not recommended for beginners. may not work.")
            visible: optionfield.selectedLabel === "Unstable"
            enabled: visible
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.9
            text: i18n.tr("Other, for testing. Only use if you know what you're doing!")
            visible: optionfield.selectedLabel === "Other"
            enabled: visible
        }

        TextField {
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            Layout.preferredWidth: parent.width * 0.9
            placeholderText: i18n.tr("Please specify...")
            visible: optionfield.selectedLabel === "Other"
            enabled: visible

            inputMethodHints: Qt.ImhDigitsOnly
            // optional inputMask if supported:
            // inputMask: "99.99"

            property string lastValid: ""

            // final-format regexp: exactly two digits, dot, two digits
            readonly property var finalRe: /^[0-9]{2}\.[0-9]{2}$/
            // relaxed regexp for intermediate typing (0–2 digits, optional dot + 0–2 digits)
            readonly property var relaxRe: /^[0-9]{0,2}(\.[0-9]{0,2})?$/

            // optional helper to check final validity
            function isFinalValid() { return finalRe.test(text); }

            onTextChanged: {
                // allow relaxed intermediate input; enforce final format elsewhere (e.g. on accept)
                if (relaxRe.test(text)) {
                    lastValid = text;
                } else {
                    text = lastValid; // revert invalid input
                }
                if (isFinalValid()) {
                    optionfield.selectedValue = text;
                } else {
                    optionfield.selectedValue = "other";
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n.tr("Channel: " + optionfield.selectedLabel) 
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            Layout.fillWidth: true
        }

        Button {
            color: theme.palette.normal.positive
            Layout.alignment: Qt.AlignHCenter
            text: i18n.tr("Install nix/home manager")
            enabled: optionfield.selectedValue != "other" && optionfield.selectedLabel != ""
            onClicked: {setupPage.handleinstallrequest(optionfield.selectedValue);}
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
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.9
            text: i18n.tr('Installing nix and home manager, this may take a few minutes — please wait.')
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
        id: report
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
            id: label0
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.9
            text: i18n.tr("placeholder")
        }

        Item {
            Layout.fillHeight: true
        }

        Button {
            id: reportbtn
            Layout.alignment: Qt.AlignHCenter
            text: i18n.tr("placeholder")
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
