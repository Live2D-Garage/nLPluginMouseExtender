
/**
 * Copyright(c) Live2D Inc. All rights reserved.
 * Licensed under the MIT License. See LICENSE file in the project root for license information.
 */
import QtCore
import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Layouts
import QtQuick.Shapes
import Qt.labs.platform as Labs
import NLPluginApi

Window {
    id: window
    width: 480
    height: width * internal.height / internal.width + 32
    minimumWidth: width
    minimumHeight: height
    maximumWidth: width
    maximumHeight: height
    visible: true
    title: Application.name
    flags: Qt.Window | Qt.FramelessWindowHint

    License {
        id: licenseWindow
    }

    // call from C++
    function autoConnect(port) {
        api.autoLaunched = true
        api.port = port
        api.open()
        window.hide()
    }

    NLPluginApi {
        id: api
        enableAutoLaunch: true
        pluginName: Application.name
        pluginDeveloper: Application.organization
        pluginVersion: Application.version
        readonly property bool closed: state === NLPluginApi.Closed
        readonly property bool available: state === NLPluginApi.Available
        onStateChanged: function (state) {
            if (state === NLPluginApi.Closed && api.autoLaunched)
                Qt.quit()
        }
        onEventReceived: function (method) {
            if (method === Method.NotifyLaunchButtonClicked)
                window.show()
        }
    }

    Settings {
        category: "api"
        property alias token: api.token
        property alias port: api.port
    }

    Settings {
        id: settings
        property string screenName
    }

    QtObject {
        id: internal
        readonly property int x: Application.screens.reduce(
                                     (x, screen) => Math.min(x,
                                                             screen.virtualX),
                                     Application.screens[0].virtualX)
        readonly property int y: Application.screens.reduce(
                                     (y, screen) => Math.min(y,
                                                             screen.virtualY),
                                     Application.screens[0].virtualY)
        readonly property int width: Application.screens.reduce(
                                         (width, screen) => Math.max(
                                             width,
                                             screen.virtualX + screen.width
                                             * screen.devicePixelRatio), x) - x
        readonly property int height: Application.screens.reduce(
                                          (height, screen) => Math.max(
                                              height,
                                              screen.virtualY + screen.height
                                              * screen.devicePixelRatio), y) - y
        readonly property var selected: Application.screens.find(
                                            screen => screen.name === settings.screenName)
    }

    Timer {
        interval: 16
        repeat: true
        running: api.available
        onTriggered: {
            const pos = Cursor.pos()
            let x = -1 + 2 * Math.max(
                    0, Math.min(
                        1,
                        (pos.x - internal.selected.virtualX) / internal.selected.width))
            let y = 1 - 2 * Math.max(
                    0, Math.min(
                        1,
                        (pos.y - internal.selected.virtualY) / internal.selected.height))
            api.send(Method.SetLiveParameterValues, {
                         "LiveParameterValues": [{
                                 "Id": "MouseX",
                                 "Value": x
                             }, {
                                 "Id": "MouseY",
                                 "Value": y
                             }]
                     })
        }
    }

    Labs.Menu {
        id: windowMenu
        Labs.MenuItem {
            text: "About"
            onTriggered: licenseWindow.show()
        }
        Labs.MenuItem {
            text: "Quit"
            onTriggered: Qt.quit()
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 32

            MouseArea {
                anchors.fill: parent
                onPressed: mouse => {
                               forceActiveFocus()
                               mouse.accepted = false
                               window.startSystemMove()
                           }
            }
            Rectangle {
                anchors.fill: parent
                color: "#222"
                border.color: "#888"
            }

            Row {
                height: parent.height
                MouseArea {
                    width: parent.height
                    height: width
                    onClicked: windowMenu.open()
                    Image {
                        anchors.centerIn: parent
                        width: 20
                        height: width
                        mipmap: true
                        smooth: true
                        source: "qrc:/resources/icon.png"
                    }
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: "white"
                    font.pixelSize: 12
                    text: Application.name
                }
            }
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                height: parent.height
                spacing: 8
                Rectangle {
                    anchors.verticalCenter: parent.verticalCenter
                    width: 12
                    height: width
                    radius: width / 2
                    color: api.closed ? "#f00" : api.available ? "#0f0" : "#ff0"
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: "Port"
                    color: "white"
                }
                TextField {
                    width: 60
                    anchors.verticalCenter: parent.verticalCenter
                    text: String(api.port)
                    validator: IntValidator {}
                    onTextEdited: api.port = text
                }
                Button {
                    anchors.verticalCenter: parent.verticalCenter
                    text: api.closed ? "Connect" : "Disconnect"
                    onClicked: api.closed ? api.open() : api.close()
                }
            }
            Row {
                anchors.right: parent.right
                height: parent.height
                MouseArea {
                    visible: api.autoLaunched
                    width: height * 1.5
                    height: parent.height
                    hoverEnabled: true
                    onClicked: window.hide()
                    Rectangle {
                        anchors.fill: parent
                        color: "#888"
                        opacity: 0.25
                        visible: parent.containsMouse
                    }
                    Shape {
                        id: hideShape
                        anchors.centerIn: parent
                        width: 10
                        ShapePath {
                            strokeColor: "#bbb"
                            startX: 0
                            startY: 0
                            PathLine {
                                x: hideShape.width
                                y: 0
                            }
                        }
                    }
                }
                MouseArea {
                    width: height * 1.5
                    height: parent.height
                    hoverEnabled: true
                    onClicked: window.close()
                    Rectangle {
                        anchors.fill: parent
                        color: "#888"
                        opacity: 0.25
                        visible: parent.containsMouse
                    }
                    Shape {
                        id: closeShape
                        anchors.centerIn: parent
                        width: 10
                        height: width
                        ShapePath {
                            strokeColor: "#bbb"
                            startX: 0
                            startY: 0
                            PathLine {
                                x: closeShape.width
                                y: closeShape.width
                            }
                        }
                        ShapePath {
                            strokeColor: "#bbb"
                            startX: 0
                            startY: closeShape.width
                            PathLine {
                                x: closeShape.width
                                y: 0
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: screenViewer
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#000"
            border.color: "#888"
            Repeater {
                model: Application.screens
                delegate: MouseArea {
                    required property var modelData
                    readonly property bool selected: modelData.name === settings.screenName
                    x: (modelData.virtualX - internal.x) * screenViewer.width / internal.width
                    y: (modelData.virtualY - internal.y) * screenViewer.height / internal.height
                    width: modelData.width * modelData.devicePixelRatio
                           * screenViewer.width / internal.width
                    height: modelData.height * modelData.devicePixelRatio
                            * screenViewer.height / internal.height
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onClicked: settings.screenName = modelData.name
                    Rectangle {
                        anchors.fill: parent
                        color: parent.containsMouse ? "#333" : "#222"
                        border.color: parent.selected ? "#fff" : "#888"
                        border.width: parent.selected ? 2 : 1
                    }
                    Column {
                        anchors.centerIn: parent
                        width: parent.width
                        Text {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                            text: modelData.name
                            color: "white"
                        }
                        Text {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            elide: Text.ElideRight
                            text: modelData.manufacturer
                            color: "white"
                        }
                    }
                }
            }
        }
    }
}
