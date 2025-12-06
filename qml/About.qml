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

    header: PageHeader {
        id: header
        title: i18n.tr('About')

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

        contentWidth: parent.width
        contentHeight: about.implicitHeight
        clip: true
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        interactive: true

        ColumnLayout {
            id: about
            spacing: units.gu(2)
            enabled: true
            visible: true
            // anchors {
            //     top: header.bottom
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
                text: i18n.tr('About Nix Manager:')
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                text: i18n.tr("Nix Manager started as a Lomiri system settings plugin, but after UX discussions it was decided to become a standalone app.")
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                textFormat: Text.RichText
                text: i18n.tr("Nix Manager is a project that began in tandem with Crackle's adoption of Nix. <br>I (ChromiumOS-Guy) developed the Python libraries Crackle uses to interact with Nix, which is how this project started. <br>There’s a forum post with more background: <br>" + '<a href="https://forums.ubports.com/topic/11067/snap-pop-crackle-....and-nix-the-future-of-crackle/8?_=1765038456437">the forum post</a>' )
                onLinkActivated: {
                    Qt.openUrlExternally(link);
                }
            }

            Label {
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.preferredWidth: parent.width * 0.9
                text: i18n.tr("This all culminated in this project, I hope you find it useful. \nNix Manager aims to make Nix more accessible, since using the terminal isn’t always enjoyable on phone/tablet form factor.")
            }
        }
    }
}