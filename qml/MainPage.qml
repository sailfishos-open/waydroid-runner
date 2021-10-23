/****************************************************************************
**
** Copyright (C) 2017-2020 Elros https://github.com/elros34
**               2020-2021 Rinigus https://github.com/rinigus
**               2012 Digia Plc and/or its subsidiary(-ies).
**
** This file is part of Waydroid Runner.
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of the copyright holder nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
**
****************************************************************************/

import QtQuick 2.0
import Sailfish.Silica 1.0
import "."

Page {
    id: root
    objectName: "mainPage"

    property bool appFinished: false
    property bool appStarted: false
    property int  nwindows: 0
    property bool settingsInitDone: false

    BusyIndicator {
        id: busyInd
        anchors.centerIn: root
        running: false
        size: BusyIndicatorSize.Large
    }

    LabelC {
        id: busyInfoMessage
        anchors.top: busyInd.bottom
        anchors.topMargin: Theme.paddingLarge
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("Starting Android session")
        visible: busyInd.running
    }

    // Start and end notification
    Image {
        anchors.centerIn: busyInd
        source: Qt.resolvedUrl("icons/waydroid.png")
        sourceSize.width: busyInd.width / 2
        visible: (!appStarted || appFinished)
    }

    Label {
        id: hintLabel
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: busyInd.bottom
        anchors.topMargin: Theme.paddingLarge
        font.pixelSize: Theme.fontSizeLarge
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        visible: text
        width: root.width - 2*Theme.horizontalPageMargin
        wrapMode: Text.WordWrap
    }

    // Connections and signal handlers
    Connections {
        target: runner
        onExit: {
            if (runner.crashed)
                hintLabel.text = qsTr("Android session crashed");
            else {
                if (runner.exitCode)
                    hintLabel.text = qsTr("Android session finished with exit code %1").arg(runner.exitCode);
                else
                    hintLabel.text = qsTr("Android session finished");
            }
            appFinished = true;
            busyInd.running = false;
        }
    }

    Component.onCompleted: {
        busyInd.running = true;
    }

    onNwindowsChanged: {
        if (nwindows > 0) {
            appStarted = true;
            busyInd.running = false;
        }
    }

    // Handling of contained application
    function windowAdded(window) {
        var windowContainerComponent = Qt.createComponent("WindowContainer.qml");
        if (windowContainerComponent.status !== Component.Ready) {
            console.warn("Error loading WindowContainer.qml: " +  windowContainerComponent.errorString());
            return;
        }

        var windowContainer = windowContainerComponent.createObject(root, {
                                                                        child: compositor.item(window),
                                                                        popup: nwindows > 0
                                                                    });

//        console.log("New window: " + windowContainer.child + " " +
//                    windowContainer.child.width + " x " + windowContainer.child.height + " / " +
//                    windowContainer.child.x + " , " + windowContainer.child.y)

        nwindows += 1;
    }

    function windowResized(window) {
        window.width = window.surface.size.width;
        window.height = window.surface.size.height;
    }

    function removeWindow(window) {
        window.destroy();
        nwindows -= 1;
//        console.log("Window destroyed")
    }
}

