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
    id: generationsPage
    property string potential_nix_generation: ""
    property bool expire_action: false

    Component {
        id: dialog
        Dialog {
            id: dialogue
            title: "placeholder"
            text: "placeholder"

            Component.onCompleted: {
                if (expire_action == false) {
                    this.title = "Delete old generations?";
                    this.text = "This will delete ALL nix generations except the current one.";
                } else {
                    this.title = "Expire old generations?";
                    this.text = "This will delete ALL home-manager generations older then the allowed timestamp (" + root.expire_generation_timestamp + ").";
                }
            }

            Button {
                text: "Delete"
                color: theme.palette.normal.positive
                onClicked: {
                    if (expire_action == false) {
                        generationList.visible = false;
                        generationList.enabled = false;
                        loadingbar.visible = true;
                        loadingbar.enabled = true;
                        loadinglabel.text = i18n.tr('Deleting nix generations, please wait.');
                        root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                        NixManagerPlugin.request_delete_old_generations(root.currentRequestId);
                    } else {
                        generationList.visible = false;
                        generationList.enabled = false;
                        loadingbar.visible = true;
                        loadingbar.enabled = true;
                        loadinglabel.text = i18n.tr('Deleting home-manager generations by timestamp, please wait.');
                        root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                        NixManagerPlugin.request_hm_expire_generations(root.currentRequestId, root.expire_generation_timestamp);
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
                        if (operation == "list_generations") {
                            generationList.visible = true;
                            generationList.enabled = true;
                            loadingbar.visible = false;
                            loadingbar.enabled = false;
                            generationList.setGenerations(result.output);
                        } else if (operation == "switch_generation") {
                            root.nix_generation = potential_nix_generation;
                            root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                            NixManagerPlugin.request_list_generations(root.currentRequestId);
                        } else if (operation == "delete_generation") {
                            generationList.visible = true;
                            generationList.enabled = true;
                            loadingbar.visible = false;
                            loadingbar.enabled = false;
                        } else if (operation == "hm_expire_generations" || operation == "delete_old_generations") {
                            root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                            NixManagerPlugin.request_list_generations(root.currentRequestId);
                        }
                        
                    } else {
                        generationList.visible = false;
                        generationList.enabled = false;
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
        title: i18n.tr('Generations')

        leadingActionBar.actions: [
            Action {
                text: i18n.tr('Back')
                iconName: 'toolkit_chevron-rtl_1gu'
                onTriggered: root.popPage()
            }
        ]
    }

    ColumnLayout {
        id: generationList
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
            id: generations_Model
        }

        Component.onCompleted: {

            root.currentRequestId = "VERSION_REQUEST_" + Date.now();
            NixManagerPlugin.request_list_generations(root.currentRequestId);
        }

        // {"id":"1","datetime":"2025-09-25 16:47:18","is_current":false}, ...

        function setGenerations(generations) {
            generations_Model.clear();
            if (!generations || !generations.length) return;
            for (var i = 0; i < generations.length; i++) {
                try {
                    var obj = JSON.parse(generations[i]);

                    if (obj.is_current == true) {
                        generations_Model.insert(0, obj);
                    } else {
                        generations_Model.append(obj);
                    }
                } catch (e) {
                    console.log("Failed to parse generations[" + i + "]: " + e);
                }
            }
        }



        // Clickable list
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: generations_Model
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
                            enabled: model.is_current == false
                            iconName: "tick"
                            property bool colorp: true

                            onTriggered: {
                                root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                                NixManagerPlugin.request_switch_generation(root.currentRequestId, model.id);
                                potential_nix_generation = model.id;
                                generationList.visible = false;
                                generationList.enabled = false;
                                loadingbar.visible = true;
                                loadingbar.enabled = true;
                                loadinglabel.text = i18n.tr('Switching generation, please wait.');
                            }
                        },
                        Action {
                            enabled: model.is_current == false
                            iconName: "delete"
                            property bool colorp: false

                            onTriggered: {
                                root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                                NixManagerPlugin.request_delete_generation(root.currentRequestId, model.id);
                                generations_Model.remove(index)
                                generationList.visible = false;
                                generationList.enabled = false;
                                loadingbar.visible = true;
                                loadingbar.enabled = true;
                                loadinglabel.text = i18n.tr('Deleting generation, please wait.');

                            }
                        }
                    ]
                    delegate: Icon {
                        name: action.iconName
                        color: action.colorp == true ? theme.palette.normal.positive : theme.palette.normal.negative
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
                        text: model.id
                        font.bold: true
                        elide: Text.ElideRight
                        wrapMode: Text.WordWrap
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHRight
                        text: i18n.tr('Is current generation')
                        elide: Text.ElideRight
                        color: theme.palette.normal.positive
                        visible: model.is_current == true ? true : false
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        RowLayout {
            Item { Layout.fillWidth: true }

            Button {
                color: theme.palette.normal.positive
                Layout.alignment: Qt.AlignVCenter
                text: i18n.tr("Delete old")
                onClicked: {
                    expire_action = false;
                    PopupUtils.open(dialog);
                } // open popup to confirm apply.
            }

            Item { Layout.fillWidth: true }

            Button {
                color: theme.palette.normal.positive
                Layout.alignment: Qt.AlignVCenter
                text: i18n.tr("Expire by timestamp")
                onClicked: {
                    expire_action = true;
                    PopupUtils.open(dialog);
                } // open popup to confirm apply.
            }

            Item { Layout.fillWidth: true }
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
            text: i18n.tr('Deleting Generation, please wait.')
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