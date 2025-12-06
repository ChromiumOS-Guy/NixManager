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
    id: applyPage

    Component {
        id: dialog
        Dialog {
            id: dialogue
            title: "Apply changes?"
            text: "This will apply all the changes you have selected."
            Button {
                text: "Apply"
                color: theme.palette.normal.positive
                onClicked: {
                    PopupUtils.close(dialogue)
                    summery.visible = false;
                    summery.enabled = false;
                    loadingbar.visible = true;
                    loadingbar.enabled = true;
                    header.leadingActionBar.visible = false;
                    header.leadingActionBar.enabled = false;
                    if (root.packages_to_delete.length > 0) {
                        root.currentRequestId = "VERSION_REQUEST_" + Date.now(); // we will start with deleting packages because it is the least likely to cause problems
                        NixManagerPlugin.request_delete_packages(root.currentRequestId, JSON.stringify(root.packages_to_delete));
                    } else if (root.packages_to_install.length > 0) {
                        var packages_to_install_processed = [];
                        for (var i = 0; i < root.packages_to_install.length; i++) {
                            try {
                                var obj = JSON.parse(root.packages_to_install[i]);
                                packages_to_install_processed.push("pkgs." + obj.name);
                            } catch (e) {
                                console.log("Failed to parse packages_to_install[" + i + "]: " + e);
                            }
                        }
                        console.log(JSON.stringify(packages_to_install_processed));
                        root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                        NixManagerPlugin.request_add_packages(root.currentRequestId, JSON.stringify(packages_to_install_processed), root.allow_insecure_pakcages); 
                        loadinglabel.text = i18n.tr('Installing packages, please wait.')
                    }
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
                        if (operation == "delete_packages") {
                            if (root.packages_to_install.length > 0) { // get to installing packages
                                var packages_to_install_processed = [];
                                for (var i = 0; i < root.packages_to_install.length; i++) {
                                    try {
                                        var obj = JSON.parse(root.packages_to_install[i]);
                                        packages_to_install_processed.push("pkgs." + obj.name);
                                    } catch (e) {
                                        console.log("Failed to parse packages_to_install[" + i + "]: " + e);
                                    }
                                }
                                root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                                NixManagerPlugin.request_add_packages(root.currentRequestId, JSON.stringify(packages_to_install_processed), root.allow_insecure_pakcages); 
                                loadinglabel.text = i18n.tr('Installing packages, please wait.')
                            } else {
                                loadingbar.visible = false;
                                loadingbar.enabled = false;
                                header.leadingActionBar.visible = true;
                                header.leadingActionBar.enabled = true;
                                if (root.packages_to_delete.length > 0) {
                                    label0.text = i18n.tr('Package deletion successful!');
                                    if (root.packages_to_install.length > 0) {
                                        label0.text = i18n.tr('Package deletion/installation successful!');
                                    }
                                }
                                label0.color = theme.palette.normal.positive;
                                reportbtn.visible = false;
                                reportbtn.enabled = false;
                                report.visible = true;
                                report.enabled = true;
                            }
                            root.packages_to_delete = [];
                        } else if (operation == "add_packages") {
                            loadingbar.visible = false;
                            loadingbar.enabled = false;
                            header.leadingActionBar.visible = true;
                            header.leadingActionBar.enabled = true;
                            if (root.packages_to_install.length > 0) {
                                label0.text = i18n.tr('Package installation successful!');
                                if (root.packages_to_delete.length > 0) {
                                    label0.text = i18n.tr('Package deletion/installation successful!');
                                }
                            }
                            root.packages_to_install = [];
                            label0.color = theme.palette.normal.positive;
                            reportbtn.visible = false;
                            reportbtn.enabled = false;
                            report.visible = true;
                            report.enabled = true;
                        }
                        
                    } else {
                        if (operation == "delete_packages") {
                            loadingbar.visible = false;
                            loadingbar.enabled = false;
                            report.visible = true;
                            report.enabled = true;
                            label0.text = i18n.tr('Package deletion failed!');
                            label0.color = theme.palette.normal.negative;
                            reportbtn.text = i18n.tr('Details');
                            reportbtn.clicked.connect(function() {report.visible = false; report.enabled = false; showerror.visible = true; showerror.enabled = true; showerror.error = result.full_error.join(' '); simpleerror.chosen = result.simple_error.join(' ');})
                            report.visible = true;
                            report.enabled = true;
                            header.leadingActionBar.visible = true;
                            header.leadingActionBar.enabled = true;
                        } else if (operation == "add_packages") {
                            loadingbar.visible = false;
                            loadingbar.enabled = false;
                            label0.text = i18n.tr('Package installation failed!');
                            label0.color = theme.palette.normal.negative;
                            reportbtn.text = i18n.tr('Details');
                            reportbtn.clicked.connect(function() {report.visible = false; report.enabled = false; showerror.visible = true; showerror.enabled = true; showerror.error = result.full_error.join(' '); simpleerror.chosen = result.simple_error.join(' ');})
                            report.visible = true;
                            report.enabled = true;
                            header.leadingActionBar.visible = true;
                            header.leadingActionBar.enabled = true;
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
        title: i18n.tr('Apply changes')

        leadingActionBar.actions: [
            Action {
                text: i18n.tr('Back')
                iconName: 'toolkit_chevron-rtl_1gu'
                onTriggered: root.popPage()
            }
        ]
    }

    ColumnLayout {
        id: summery
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

        Label {
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHRight
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.9
            color: theme.palette.normal.base
            text: i18n.tr('Packages to install:')
        }

        ListModel {
            id: install_packages_Model
        }

        ListModel {
            id: delete_packages_Model
        }

        function stripPkgPrefix(s) {
            if (!s) return s;
            if (s.startsWith("pkgs.")) return s.slice("pkgs.".length);
            if (s.startsWith("nixpkgs.")) return s.slice("nixpkgs.".length);
            return s;
        }

        Component.onCompleted: {
            install_packages_Model.clear();
            if (root.packages_to_install || root.packages_to_install.length) {
                for (var i = 0; i < root.packages_to_install.length; i++) {
                    try {
                        var obj = JSON.parse(root.packages_to_install[i]);
                        // obj contains keys like name, summary, last_updated (or others)
                        var name = obj.name ? obj.name : "";
                        var summary = obj.summary ? obj.summary : "";
                        var last_updated = obj.last_updated ? obj.last_updated : "";
                        install_packages_Model.append({ name: name, summary: summary, last_updated: last_updated, raw: obj });
                    } catch (e) {
                        console.log("Failed to parse packages_to_install[" + i + "]: " + e);
                    }
                }
            }

            delete_packages_Model.clear();
            if (root.packages_to_delete || root.packages_to_delete.length) {
                for (var i = 0; i < root.packages_to_delete.length; i++) {
                    try {
                        delete_packages_Model.append({ name: packages_to_delete[i]});
                    } catch (e) {
                        console.log("Failed to parse packages_to_delete[" + i + "]: " + e);
                    }
                }
            }
        }

        // Clickable list
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: install_packages_Model
            visible: true
            enabled: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            interactive: true

            delegate: ListItem {
                id: delegateRoot
                //width: parent.width
                height: Math.max(contentColumn.implicitHeight, units.gu(7))

                leadingActions: ListItemActions {
                    actions: [
                        Action {
                            iconName: "delete"

                            onTriggered: {
                                if (root.packages_to_install.includes(JSON.stringify(model.raw))) {
                                    var idx = root.packages_to_install.indexOf(JSON.stringify(model.raw))
                                    if (idx !== -1) {
                                        root.packages_to_install.splice(idx, 1)
                                        root.packages_to_install = root.packages_to_install // trigger QML change
                                    }
                                    install_packages_Model.remove(index)
                                }
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
                ColumnLayout {
                    id: contentColumn
                    anchors {
                        top: parent.top
                        // left: parent.swipe.complete == true ? parent.swipe.leftItem.right : parent.left
                        // right: parent.right
                        bottom: parent.bottom
                    }
                    width: parent.width
                    //x: parent.swipe.leftItem != null ? (swipe.position * parent.swipe.leftItem.width) : this.x

                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHRight
                        text: model.name
                        font.bold: true
                        wrapMode: Text.WordWrap
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHRight
                        // choose summary if non-empty, otherwise last_updated, otherwise empty
                        property string chosen: (model.summary && model.summary !== "") ? model.summary :
                                                (model.last_updated && model.last_updated !== "") ? model.last_updated : ""
                        text: chosen
                        elide: Text.ElideRight
                        wrapMode: Text.WordWrap
                        visible: chosen !== ""
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHRight
                        // choose last_updated if non-empty and summary is non-empty, otherwise empty
                        property string chosen: (model.summary && model.summary !== "") && (model.last_updated && model.last_updated !== "") ? model.last_updated : ""
                        text: chosen
                        elide: Text.ElideRight
                        wrapMode: Text.WordWrap
                        visible: chosen !== ""
                    }
                }
            }
        }

        Label {
            Layout.alignment: Qt.AlignHCenter
            horizontalAlignment: Text.AlignHRight
            wrapMode: Text.WordWrap
            Layout.preferredWidth: parent.width * 0.9
            color: theme.palette.normal.base
            text: i18n.tr('Packages to remove:')
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: delete_packages_Model
            visible: true
            enabled: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            interactive: true

            delegate: ListItem {
                id: delegateRoot
                //width: parent.width
                height: Math.max(contentColumn.implicitHeight, units.gu(7))

                leadingActions: ListItemActions {
                    actions: [
                        Action {
                            iconName: "delete"

                            onTriggered: {
                                if (root.packages_to_delete.includes(model.name)) {
                                    var idx = root.packages_to_delete.indexOf(model.name)
                                    if (idx !== -1) {
                                        root.packages_to_delete.splice(idx, 1)
                                        root.packages_to_delete = root.packages_to_delete // trigger QML change
                                    }
                                    delete_packages_Model.remove(index)
                                }
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
                ColumnLayout {
                    id: contentColumn
                    anchors {
                        top: parent.top
                        // left: parent.swipe.complete == true ? parent.swipe.leftItem.right : parent.left
                        // right: parent.right
                        bottom: parent.bottom
                    }
                    width: parent.width
                    //x: parent.swipe.leftItem != null ? (swipe.position * parent.swipe.leftItem.width) : this.x

                    Label {
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHRight
                        text: summery.stripPkgPrefix(model.name)
                        font.bold: true
                        elide: Text.ElideRight
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }

        Button {
            color: theme.palette.normal.positive
            Layout.alignment: Qt.AlignHCenter
            text: i18n.tr("Apply")
            enabled: (root.packages_to_delete.length != 0) || (root.packages_to_install.length != 0)
            onClicked: {PopupUtils.open(dialog)} // open popup to confirm apply.
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
            text: i18n.tr('Removing packages, please wait.')
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