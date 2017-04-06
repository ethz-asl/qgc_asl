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
    property var _linkManager:      QGroundControl.linkManager

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
                    QGCLabel { text: mainWindow.activeCommText }
                }

                Button {
                    id: commSwitchButton
                    width: switchButtonText.width + 10
                    height: switchButtonText.height + 10
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: mainWindow.multipleLinks
                    Text {
                        id: switchButtonText
                        text: mainWindow.switchCommText
                        font.pointSize: 8
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    onClicked: {
                        if (QGroundControl.linkManager.switchSatcomClick()) {
                            mainWindow.satcomOpacity = 1.0
                            mainWindow.activeCommText = "Satcom Active"
                            mainWindow.switchCommText = "Switch to Telemetry"
                        }
                        else {
                            mainWindow.satcomOpacity = 0.5
                            mainWindow.activeCommText = "Telemetry Active"
                            mainWindow.switchCommText = "Switch to Satcom"
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
            opacity:            mainWindow.satcomOpacity
            color:              qgcPal.buttonText
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
