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
    id: channelsPage
    property bool update_action: false

    Component {
        id: dialog
        Dialog {
            id: dialogue
            title: "placeholder"
            text: "placeholder"

            property string name: ""
            property string url: ""

            Component.onCompleted: {
                if (update_action == false) {
                    this.title = "Add channel?";
                    this.text = "This will add a channel.";
                } else {
                    this.title = "Update channels?";
                    this.text = "This will update channels, (like apt update).";
                }
            }

            // Channel name row
            RowLayout {
                visible: !update_action
                enabled: !update_action
                spacing: units.gu(1)
                Label {
                    text: i18n.tr("Channel name:")
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredWidth: parent.width * 0.35
                    wrapMode: Text.WordWrap
                }
                TextField {
                    text: name
                    Layout.fillWidth: true
                    property string lastValid: ""
                    // allow any characters except whitespace
                    readonly property var relaxRe: /^[^\s]*$/

                    onTextChanged: {
                        if (relaxRe.test(text)) {
                            lastValid = text;
                            name = text
                        } else {
                            // revert to previous valid value if the new text contains whitespace
                            text = lastValid;
                        }
                    }

                    onAccepted: {
                        if (relaxRe.test(text)) {
                            name = text.replace(/\s+/g, "") // final sanitize just in case
                        } 
                    }

                }
            }

            // Channel URL row
            RowLayout {
                visible: !update_action
                enabled: !update_action
                spacing: units.gu(1)
                Label {
                    text: i18n.tr("Channel url:")
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

            Button {
                text: update_action == false ? "Add" : "Update"
                color: theme.palette.normal.positive
                enabled: (name !=  "" && url != "") || update_action == true
                onClicked: {
                    if (update_action == false) {
                        root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                        NixManagerPlugin.request_add_channel(root.currentRequestId, url, name);
                        channelList.visible = false;
                        channelList.enabled = false;
                        loadingbar.visible = true;
                        loadingbar.enabled = true;
                        loadinglabel.text = i18n.tr('Adding channel, please wait.');
                    } else {
                        root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                        NixManagerPlugin.request_update_channels(root.currentRequestId);
                        channelList.visible = false;
                        channelList.enabled = false;
                        loadingbar.visible = true;
                        loadingbar.enabled = true;
                        loadinglabel.text = i18n.tr('Updating channels, please wait.');
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
                        if (operation == "list_channels") {
                            channelList.visible = true;
                            channelList.enabled = true;
                            loadingbar.visible = false;
                            loadingbar.enabled = false;
                            channelList.setGenerations(result.output);
                        } else if (operation == "add_channel") {
                            root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                            NixManagerPlugin.request_list_channels(root.currentRequestId);
                        } else if (operation == "remove_channel") {
                            channelList.visible = true;
                            channelList.enabled = true;
                            loadingbar.visible = false;
                            loadingbar.enabled = false;
                        } else if (operation == "update_channels") {
                            channelList.visible = true;
                            channelList.enabled = true;
                            loadingbar.visible = false;
                            loadingbar.enabled = false;
                        }
                        
                    } else {
                        channelList.visible = false;
                        channelList.enabled = false;
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
        title: i18n.tr('Channels')

        leadingActionBar.actions: [
            Action {
                text: i18n.tr('Back')
                iconName: 'toolkit_chevron-rtl_1gu'
                onTriggered: root.popPage()
            }
        ]
    }

    ColumnLayout {
        id: channelList
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
            id: channels_Model
        }

        Component.onCompleted: {

            root.currentRequestId = "VERSION_REQUEST_" + Date.now();
            NixManagerPlugin.request_list_channels(root.currentRequestId);
        }

        //   {"name": "name", "url": "url"}

        function setGenerations(channels) {
            channels_Model.clear();
            if (!channels || !channels.length) return;
            for (var i = 0; i < channels.length; i++) {
                try {
                    var obj = JSON.parse(channels[i]);
                    channels_Model.append(obj);
                } catch (e) {
                    console.log("Failed to parse channels[" + i + "]: " + e);
                }
            }
        }



        // Clickable list
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: channels_Model
            visible: true
            enabled: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            interactive: true

            delegate: ListItem {
                id: delegateRoot
                //width: parent.width

                leadingActions: ListItemActions {
                    actions: [
                        Action {
                            iconName: "delete"
                            property bool colorp: false
                            enabled: model.name != "home-manager" && model.name != "nixpkgs"

                            onTriggered: {
                                root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                                NixManagerPlugin.request_remove_channel(root.currentRequestId, model.name);
                                channels_Model.remove(index)
                                channelList.visible = false;
                                channelList.enabled = false;
                                loadingbar.visible = true;
                                loadingbar.enabled = true;
                                loadinglabel.text = i18n.tr('Removing channel, please wait.');

                            }
                        }
                    ]
                    delegate: Icon {
                        name: action.iconName
                        color: theme.palette.normal.negative
                        anchors.centerIn: parent
                    }
                }

                // Show name on top, and a single secondary label (summary OR last_updated)
                Column {
                    anchors {
                        top: parent.top
                        // left: parent.swipe.complete == true ? parent.swipe.leftItem.right : parent.left
                        // right: parent.right
                        bottom: parent.bottom
                    }
                    //x: parent.swipe.leftItem != null ? (swipe.position * parent.swipe.leftItem.width) : this.x

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHRight
                        text: model.name
                        font.bold: true
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHRight
                        text: model.url
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHRight
                        text: i18n.tr('Protected channel.')
                        elide: Text.ElideRight
                        visible:  model.name == "home-manager" || model.name == "nixpkgs"
                    }
                }
            }
        }

        RowLayout {
            Item { Layout.fillWidth: true }

            Button {
                color: theme.palette.normal.positive
                Layout.alignment: Qt.AlignVCenter
                text: i18n.tr("Add")
                onClicked: {
                    update_action = false;
                    PopupUtils.open(dialog);
                } // open popup to confirm apply.
            }

            Item { Layout.fillWidth: true }

            Button {
                color: theme.palette.normal.positive
                Layout.alignment: Qt.AlignVCenter
                text: i18n.tr("Update")
                onClicked: {
                    update_action = true;
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