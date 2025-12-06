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
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import Lomiri.Components 1.3
import NixManagerPlugin 1.0

MainView {
    id: root
    objectName: 'mainView'
    applicationName: 'nixmanager.chromiumos-guy'
    automaticOrientation: true

    property string currentRequestId: ""
    property string hm_version: ""
    property string nix_generation: ""
    property var packages_to_delete: []
    property var packages_to_install: []
    // settings
    Settings {
        id: s
        property bool allow_insecure_packages: false
        property bool enable_local_search: false
        property string search_api_url: "https://search.devbox.sh"
        property int api_timeout: 10
        property string expire_generation_timestamp: "-30 days"
    }

    property alias allow_insecure_pakcages: s.allow_insecure_packages
    property alias enable_local_search: s.enable_local_search
    property alias search_api_url: s.search_api_url
    property alias api_timeout: s.api_timeout
    property alias expire_generation_timestamp: s.expire_generation_timestamp // expire older then a month

    function loadQml(path) {
        var comp = Qt.createComponent(path);
        if (comp.status === Component.Ready) {
            var obj = comp.createObject(stack);
            if (!obj) { console.error("createObject failed"); return; }
            Qt.callLater(function() { stack.push(obj); });
            return;
        }
        if (comp.status === Component.Error) { console.error(comp.errorString()); return; }
        comp.statusChanged.connect(function onStatusChanged() {
            if (comp.status === Component.Ready) {
                comp.statusChanged.disconnect(onStatusChanged);
                var obj = comp.createObject(stack);
                if (!obj) { console.error("createObject failed"); return; }
                Qt.callLater(function() { stack.push(obj); });
            } else if (comp.status === Component.Error) {
                comp.statusChanged.disconnect(onStatusChanged);
                console.error(comp.errorString());
            }
        });
    }


    function popPage() {
        stack.pop()
    }

     Connections {
        target: NixManagerPlugin
        
        // This handler fires for *all* completed operations
        onOperation_result: (resultJson, receivedId, operation) => {
            
            // 3. Match the ID to ensure it's the result we are waiting for
            if (receivedId === receivedId) { // bypass.
                console.log("Received result for requested ID:", receivedId);
                
                // --- Process the resultJson string here ---
                try {
                    const result = JSON.parse(resultJson);
                    if (result.full_error != "") {
                        console.error(result.full_error.join(' '));
                    }
                    // Example processing:
                    if (result.success) {
                        if (operation == "hm_version") {
                            root.hm_version = result.output.join(' ');
                        } else if (operation == "list_generations") {
                            for (var i = 0; i < result.output.length; i++) {
                                var item = JSON.parse(result.output[i]);      // parse JSON string -> object
                                if (item.is_current) {
                                    root.nix_generation = item.id;          // assign current id
                                    break;
                                }
                            }
                        } else if (operation == "search_packages") {
                            console.debug(resultJson);
                            searchbusy.running = false;
                            searchpackages.setPackages(result.output);
                        }
                    } else {
                        if (operation == "detect_nix_home_manager") { // redirect to Setup if no installation is found.
                            allow_insecure_pakcages = false; // also save into settings when implemented!
                            loadQml("Setup.qml");
                        } else if (operation == "hm_version") {
                            root.hm_version = "!!could not find version!!";
                        } else if (operation == "list_generations") {
                            root.nix_generation = "!!could not find generations!!";
                        } else if (operation == "search_packages") {
                            console.debug(resultJson);
                            searchbusy.running = false;
                            searchpackages.setPackages(result.output);
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

    StackView {
        id: stack
        anchors.fill: parent
        initialItem: mainPage
        pushEnter: Transition { /* optional animations */ }
        popExit: Transition { /* optional animations */ }
    }



     Page {
        id: mainPage

        Component.onCompleted: {
            root.currentRequestId = "VERSION_REQUEST_" + Date.now();
            NixManagerPlugin.request_detect_nix_home_manager(root.currentRequestId); // check if there is an installation and redirect to setup if false
            root.currentRequestId = "VERSION_REQUEST_" + Date.now();
            NixManagerPlugin.request_hm_version(root.currentRequestId); // check home manager version
            root.currentRequestId = "VERSION_REQUEST_" + Date.now();
            NixManagerPlugin.request_list_generations(root.currentRequestId); 
        }

        header: PageHeader {
            id: header

            title: i18n.tr('Nix Manager')

            TextField {
                Layout.fillWidth: true
                anchors {
                    margins: units.gu(1)
                    top: parent.top
                    bottom: parent.bottom
                    right: searchbtn.left
                    left: parent.left
                }
                id: searchbar
                visible: false
                enabled: false
                // Fires when Enter/Return is pressed (physical or virtual)
                onAccepted: {
                    root.currentRequestId = "VERSION_REQUEST_" + Date.now();
                    NixManagerPlugin.request_search_packages(root.currentRequestId, text, enable_local_search, search_api_url, api_timeout);
                    searchbusy.running = true;
                }
            }


            Icon {
                anchors {
                    margins: units.gu(1)
                    top: parent.top
                    bottom: parent.bottom
                    right: parent.right
                }
                visible: true
                enabled: true
                id: searchbtn
                name: "toolkit_input-search" // Lomiri search icon
                color: LomiriColors.warmGrey  // Set icon color
                anchors.verticalCenter: parent.verticalCenter

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (mainmenu.visible && mainmenu.enabled) {
                            mainmenu.visible = false;
                            mainmenu.enabled = false;
                            searchpackages.visible = true;
                            searchpackages.enabled = true;
                            searchbar.visible = true;
                            searchbar.enabled = true;
                            header.title = i18n.tr('')
                            parent.name = "toolkit_cross" // Lomiri cancel icon
                        } else {
                            mainmenu.visible = true;
                            mainmenu.enabled = true;
                            searchpackages.visible = false;
                            searchpackages.enabled = false;
                            searchbar.visible = false;
                            searchbar.enabled = false;
                            searchbar.text = i18n.tr('')
                            packagesModel.clear();
                            header.title = i18n.tr('Nix Manager')
                            parent.name = "toolkit_input-search" // Lomiri search icon
                        }
                    }
                }
            }
        }

        ColumnLayout {
            id: mainmenu
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
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                text: i18n.tr('home-manager\n' + root.hm_version)
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                text: i18n.tr('Nix Generation ' + root.nix_generation)
            }

            Label {
                enabled: (root.packages_to_delete.length + root.packages_to_install.length) > 0
                visible: (root.packages_to_delete.length + root.packages_to_install.length) > 0
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                text: i18n.tr((root.packages_to_delete.length + root.packages_to_install.length) + ' pending changes')
            }

            ListItem {       
                id: applybtn 
                enabled: (root.packages_to_delete.length + root.packages_to_install.length) > 0
                visible: (root.packages_to_delete.length + root.packages_to_install.length) > 0
                width: mainmenu.width
                height: units.gu(6)

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: units.gu(1)
                    text: "Apply"
                    color: theme.palette.normal.baseText
                }

                MouseArea { 
                    anchors.fill: parent
                    onClicked: {
                        root.loadQml("Apply_changes.qml");
                    }
                }

                Icon {
                    anchors {
                        margins: units.gu(1)
                        top: parent.top
                        bottom: parent.bottom
                        right: parent.right
                    }
                    name: "toolkit_chevron-ltr_1gu"
                    color: LomiriColors.warmGrey  // Set icon color
                    anchors.verticalCenter: parent.verticalCenter
                }
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHRight
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                color: theme.palette.normal.base
                text: i18n.tr('Management:')
            }

            ListModel {
                id: optionsModel0
                ListElement { label: "Installed packages"; value: "installed_packages" }
                ListElement { label: "Generations"; value: "generations" }
            }

            ListView {
                id: optionsView0
                model: optionsModel0
                width: parent.width
                height: optionsModel0.count * units.gu(6)
                clip: true
                interactive: false            // disables flick/drag
                boundsBehavior: Flickable.StopAtBounds

                delegate: ListItem {                      // use Item if ListItem unavailable
                    width: optionsView0.width
                    height: units.gu(6)

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: units.gu(1)
                        text: label
                        color: theme.palette.normal.baseText
                    }

                    MouseArea { 
                        anchors.fill: parent
                        onClicked: {
                            if (model.value == "installed_packages") {root.loadQml("Installed_packages.qml");}
                            if (model.value == "generations") {root.loadQml("Generations.qml");}
                        }
                    }

                    Icon {
                        anchors {
                            margins: units.gu(1)
                            top: parent.top
                            bottom: parent.bottom
                            right: parent.right
                        }
                        name: "toolkit_chevron-ltr_1gu"
                        color: LomiriColors.warmGrey  // Set icon color
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHRight
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                color: theme.palette.normal.base
                text: i18n.tr('Settings:')
            }

            ListModel {
                id: optionsModel1
                ListElement { label: "Channels"; value: "channels" }
                ListElement { label: "Advanced Settings"; value: "advanced_settings" }
                ListElement { label: "About"; value: "about" }
            }

            ListView {
                id: optionsView1
                model: optionsModel1
                width: parent.width
                height: optionsModel1.count * units.gu(6)
                clip: true
                interactive: false            // disables flick/drag
                boundsBehavior: Flickable.StopAtBounds


                delegate: ListItem {                      // use Item if ListItem unavailable
                    width: optionsView1.width
                    height: units.gu(6)

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: units.gu(1)
                        text: label
                        color: theme.palette.normal.baseText
                    }

                    MouseArea { anchors.fill: parent
                        onClicked: {
                            if (model.value == "channels") {root.loadQml("Channels.qml");}
                            if (model.value == "advanced_settings") {root.loadQml("Advanced_Settings.qml");}
                            if (model.value == "about") {root.loadQml("About.qml");}
                        }
                    }

                    Icon {
                        anchors {
                            margins: units.gu(1)
                            top: parent.top
                            bottom: parent.bottom
                            right: parent.right
                        }
                        name: "toolkit_chevron-ltr_1gu"
                        color: LomiriColors.warmGrey  // Set icon color
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }

        ColumnLayout {
            id: searchpackages
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

            BusyIndicator {
                id: searchbusy
                running: false      // start animation
                visible: this.running == true
                Layout.alignment: Qt.AlignHCenter
            }

            ListModel {
                id: packagesModel
            }

            function setPackages(resultOutput) {
                packagesModel.clear();
                if (!resultOutput || !resultOutput.length) return;
                for (var i = 0; i < resultOutput.length; i++) {
                    try {
                        var obj = JSON.parse(resultOutput[i]);
                        // obj contains keys like name, summary, last_updated (or others)
                        var name = obj.name ? obj.name : "";
                        var summary = obj.summary ? obj.summary : "";
                        var last_updated = obj.last_updated ? obj.last_updated : "";
                        packagesModel.append({ name: name, summary: summary, last_updated: last_updated, raw: obj });
                    } catch (e) {
                        console.log("Failed to parse result.output[" + i + "]: " + e);
                    }
                }
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHRight
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                color: theme.palette.normal.base
                visible: searchbusy.running == false && packagesModel.count > 0
                enabled: searchbusy.running == false && packagesModel.count > 0
                text: i18n.tr('Search results: ' + packagesModel.count)
            }

            // Clickable list
            ListView {
                id: packageList
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: packagesModel
                visible: searchbusy.running == false
                enabled: searchbusy.running == false
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
                                iconName: root.packages_to_install.includes(JSON.stringify(model.raw)) ? "delete" : "tick"
                                enabled: root.packages_to_delete.includes(model.name) == false

                                onTriggered: {
                                    if (root.packages_to_install.includes(JSON.stringify(model.raw))) {
                                        var idx = packages_to_install.indexOf(JSON.stringify(model.raw))
                                        if (idx !== -1) {
                                            packages_to_install.splice(idx, 1)
                                            packages_to_install = packages_to_install // trigger QML change
                                        }
                                        //parent.name = "toolkit_tick"

                                    } else {
                                        root.packages_to_install.push(JSON.stringify(model.raw))
                                        packages_to_install = packages_to_install // trigger QML change
                                        //parent.name = "toolkit_cross"
                                    }
                                }
                            }
                        ]
                        delegate: Icon {
                            name: action.iconName
                            color: root.packages_to_install.includes(JSON.stringify(model.raw)) ? theme.palette.normal.negative : theme.palette.normal.positive
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
                            elide: Text.ElideRight
                            wrapMode: Text.WordWrap
                        }

                        Label {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignHCenter
                            horizontalAlignment: Text.AlignHRight
                            color: root.packages_to_install.includes(JSON.stringify(model.raw)) == true ? theme.palette.normal.positive : (root.packages_to_delete.includes(model.name) == true ? theme.palette.normal.negative : LomiriColors.warmGrey)
                            text: root.packages_to_install.includes(JSON.stringify(model.raw)) == true ? i18n.tr('Marked for installation') : (root.packages_to_delete.includes(model.name) == true ? i18n.tr('Marked for removal') : i18n.tr('Unknown State'))
                            elide: Text.ElideRight
                            visible: root.packages_to_install.includes(JSON.stringify(model.raw)) == true || root.packages_to_delete.includes(model.name) == true
                            wrapMode: Text.WordWrap
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
                            visible: (chosen !== "" && root.packages_to_install.includes(JSON.stringify(model.raw)) == false) && root.packages_to_delete.includes(model.name) == false
                            wrapMode: Text.WordWrap
                        }

                        Label {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignHCenter
                            horizontalAlignment: Text.AlignHRight
                            // choose last_updated if non-empty and summary is non-empty, otherwise empty
                            property string chosen: (model.summary && model.summary !== "") && (model.last_updated && model.last_updated !== "") ? model.last_updated : ""
                            text: chosen
                            elide: Text.ElideRight
                            visible: (chosen !== "" && root.packages_to_install.includes(JSON.stringify(model.raw)) == false) && root.packages_to_delete.includes(model.name) == false
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }
    }
}

    
