
/**
 * Copyright(c) Live2D Inc. All rights reserved.
 * Licensed under the MIT License. See LICENSE file in the project root for license information.
 */
import QtQuick
import QtQuick.Controls.Fusion
import NLPluginApi

Window {
    id: root
    width: 560
    minimumWidth: 240
    height: 400
    color: palette.window
    title: "Licenses"

    SplitView {
        anchors.fill: parent

        ListView {
            id: listView
            SplitView.minimumWidth: 120
            SplitView.fillHeight: true
            boundsBehavior: Flickable.StopAtBounds
            spacing: 4
            model: ListModel {
                ListElement {
                    name: "LICENSE"
                    path: ":/LICENSE"
                }
                ListElement {
                    name: "Qt"
                    path: ":/licenses/qt.txt"
                }
            }
            highlight: Rectangle {
                color: palette.highlight
                opacity: 0.5
            }
            delegate: MouseArea {
                width: parent.width
                height: 24
                onClicked: listView.currentIndex = index
                Text {
                    anchors.fill: parent
                    padding: 4
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    text: name
                    color: palette.text
                }
            }
        }

        Flickable {
            SplitView.fillWidth: true
            SplitView.fillHeight: true
            contentHeight: textArea.height
            boundsBehavior: Flickable.StopAtBounds
            TextArea {
                id: textArea
                width: parent.width
                color: palette.text
                wrapMode: Text.Wrap
                readOnly: true
                selectByMouse: true
                text: TextReader.read(listView.model.get(
                                          listView.currentIndex).path)
            }
        }
    }
}
