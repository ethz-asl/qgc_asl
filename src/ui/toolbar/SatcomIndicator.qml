/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


import QtQuick          2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts  1.2

import QGroundControl                       1.0
import QGroundControl.Controls              1.0
import QGroundControl.MultiVehicleManager   1.0
import QGroundControl.ScreenTools           1.0
import QGroundControl.Palette               1.0

//-------------------------------------------------------------------------
//-- Satcom Indicator

Item {
    property var _activeVehicle:    QGroundControl.multiVehicleManager.activeVehicle

    property real   satcomOpacity:      _activeVehicle.satcomActive ? 1.0 : 0.5
    property string activeCommText:     _activeVehicle.satcomActive ? "Satcom Active" : "Telemetry Active"
    property string switchCommText:     _activeVehicle.satcomActive ? "Switch to Telemetry" : "Switch to Satcom"


    width:          satcomRow.width * 1.1
    anchors.top:    parent.top
    anchors.bottom: parent.bottom
    visible:        mainWindow.highLatencyCheck

    Component {
        id: satcomInfo

        Rectangle {
            width:  satcomCol.width   + ScreenTools.defaultFontPixelWidth  * 3
            height: satcomCol.height  + ScreenTools.defaultFontPixelHeight * 2
            radius: ScreenTools.defaultFontPixelHeight * 0.5
            color:  qgcPal.window
            border.color:   qgcPal.text

            Column {
                id:                 satcomCol
                spacing:            ScreenTools.defaultFontPixelHeight * 0.5
                width:              Math.max(satcomGrid.width, satcomLabel.width)
                anchors.margins:    ScreenTools.defaultFontPixelHeight
                anchors.centerIn:   parent

                QGCLabel {
                    id:             satcomLabel
                    text:           qsTr("Communication Info")
                    font.family:    ScreenTools.demiboldFontFamily
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                GridLayout {
                    id:                 satcomGrid
                    anchors.margins:    ScreenTools.defaultFontPixelHeight
                    columnSpacing:      ScreenTools.defaultFontPixelWidth
                    anchors.horizontalCenter: parent.horizontalCenter
                    QGCLabel { text: activeCommText }
                }

                Button {
                    id: commSwitchButton
                    width: switchButtonText.width + 10
                    height: switchButtonText.height + 10
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: mainWindow.multipleLinks
                    Text {
                        id: switchButtonText
                        text: switchCommText
                        font.pointSize: 8
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    onClicked: {
                        if (_activeVehicle.switchSatcomClick()) {
                            satcomOpacity = 1.0
                            activeCommText = "Satcom Active"
                            switchCommText = "Switch to Telemetry"
                        }
                        else {
                            satcomOpacity = 0.5
                            activeCommText = "Telemetry Active"
                            switchCommText = "Switch to Satcom"
                        }
                    }
                }
            }

            Component.onCompleted: {
                var pos = mapFromItem(toolBar, centerX - (width / 2), toolBar.height)
                x = pos.x
                y = pos.y + ScreenTools.defaultFontPixelHeight
            }
        }
    }

    Row {
        id:             satcomRow
        anchors.top:    parent.top
        anchors.bottom: parent.bottom
        spacing:        ScreenTools.defaultFontPixelWidth

        QGCColoredImage {
            width:              height
            anchors.top:        parent.top
            anchors.bottom:     parent.bottom
            sourceSize.height:  height
            source:             "/qmlimages/SatPlane.svg"
            fillMode:           Image.PreserveAspectFit
            opacity:            satcomOpacity
            color:              qgcPal.buttonText
        }
    }

    Connections {
        target: _activeVehicle
        onSatcomActiveChanged: {
            satcomOpacity = _activeVehicle.satcomActive ? 1.0 : 0.5
            activeCommText = _activeVehicle.satcomActive ? "Satcom Active" : "Telemetry Active"
            switchCommText = _activeVehicle.satcomActive ? "Switch to Telemetry" : "Switch to Satcom"
        }
    }

    MouseArea {
        anchors.fill:   parent
        onClicked: {
            var centerX = mapToItem(toolBar, x, y).x + (width / 2)
            mainWindow.showPopUp(satcomInfo, centerX)
        }
    }
}
