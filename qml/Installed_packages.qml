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
    id: installedpackagesPage

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
                        if (operation == "read_packages") {
                            packageList.setPackages(result.output);
                        }
                        
                    } else {
                        if (operation == "read_packages") {
                            packageList.visible = false;
                            packageList.enabled = false;
                            showerror.visible = true;
                            showerror.enabled = true;
                            simpleerror.chosen = result.simple_error.join(' ');
                            showerror.error = result.full_error.join(' ');
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
        title: i18n.tr('Installed packages')

        leadingActionBar.actions: [
            Action {
                text: i18n.tr('Back')
                iconName: 'toolkit_chevron-rtl_1gu'
                onTriggered: root.popPage()
            }
        ]
    }

    ColumnLayout {
        id: packageList
        spacing: units.gu(2)
        enabled: true
        visible: true
        anchors {
            topMargin: units.gu(2)
            bottomMargin: units.gu(2)
            top: header.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        ListModel {
            id: installed_packages_Model
        }

        Component.onCompleted: {

            root.currentRequestId = "VERSION_REQUEST_" + Date.now();
            NixManagerPlugin.request_read_packages(root.currentRequestId);
        }

        function stripPkgPrefix(s) {
            if (!s) return s;
            if (s.startsWith("pkgs.")) return s.slice("pkgs.".length);
            if (s.startsWith("nixpkgs.")) return s.slice("nixpkgs.".length);
            return s;
        }

        function setPackages(installed_packages) {
            installed_packages_Model.clear();
            if (!installed_packages || !installed_packages.length) return;
            for (var i = 0; i < installed_packages.length; i++) {
                try {

                    installed_packages_Model.append({ name: installed_packages[i]});
                } catch (e) {
                    console.log("Failed to parse installed_packages[" + i + "]: " + e);
                }
            }
        }



        // Clickable list
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: installed_packages_Model
            visible: true
            enabled: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            interactive: true

            delegate: ListItem {
                id: delegateRoot
                //width: parent.width
                height: contentColumn.implicitHeight

                leadingActions: ListItemActions {
                    actions: [
                        Action {
                            iconName: root.packages_to_delete.includes(model.name) ? "toolkit_cross" : "delete"

                            onTriggered: {
                                if (root.packages_to_delete.includes(model.name)) {
                                    var idx = root.packages_to_delete.indexOf(model.name)
                                    if (idx !== -1) {
                                        root.packages_to_delete.splice(idx, 1)
                                        root.packages_to_delete = root.packages_to_delete // trigger QML change
                                    }
                                    //parent.name = "toolkit_tick"

                                } else {
                                    root.packages_to_delete.push(model.name)
                                    root.packages_to_delete = root.packages_to_delete // trigger QML change
                                    //parent.name = "toolkit_cross"
                                }
                            }
                        }
                    ]
                    delegate: Icon {
                        name: action.iconName
                        color: root.packages_to_delete.includes(model.name) ? LomiriColors.warmGrey : theme.palette.normal.negative
                        anchors.centerIn: parent
                    }
                }

                // Show name on top, and a single secondary label (summary OR last_updated)
                ColumnLayout {
                    id: contentColumn
                    anchors {
                        top: parent.top
                        // left: parent.swipe.complete == true ? parent.swipe.leftItem.right : parent.left
                        // right: parent.right
                        bottom: parent.bottom
                    }
                    //x: parent.swipe.leftItem != null ? (swipe.position * parent.swipe.leftItem.width) : this.x
                    width: parent.width

                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHRight
                        text: packageList.stripPkgPrefix(model.name)
                        font.bold: true
                        elide: Text.ElideRight
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHRight
                        color: theme.palette.normal.negative
                        text: i18n.tr('Marked for removal')
                        elide: Text.ElideRight
                        visible: root.packages_to_delete.includes(model.name) == true
                        wrapMode: Text.WordWrap
                    }
                }
            }
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