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
//-- GSM Status Indicator
Item {
    anchors.top:    parent.top
    anchors.bottom: parent.bottom
    width:          gsmIndicatorRow.width

    property var _activeVehicle: QGroundControl.multiVehicleManager.activeVehicle
    readonly property int excellent: 3
    readonly property int good:      2
    readonly property int fair:      1
    readonly property int bad:       0
    readonly property int _2G:       2
    readonly property int _3G:       3
    readonly property int _4G:       4


    function getQualityLevel(value, threshold_excellent, threshold_good, threshold_fair) {
        if (value > threshold_excellent) {
            return excellent
        }
        if (value > threshold_good) {
            return good
        }
        if (value > threshold_fair) {
            return fair
        }
        return bad
    }


    function getMainTextColor() {
        if (_activeVehicle) {
            var lowest_quality = excellent

            if (_activeVehicle.gsm.linkType.value === _2G) {
                lowest_quality = Math.min(lowest_quality,
                                          getQualityLevel(_activeVehicle.gsm.rssi.value,
                                                          _activeVehicle.gsm.rssiExcellentThreshold.value,
                                                          _activeVehicle.gsm.rssiGoodThreshold.value,
                                                          _activeVehicle.gsm.rssiFairThreshold.value))
            } else if (_activeVehicle.gsm.linkType.value === _3G) {
                lowest_quality = Math.min(lowest_quality,
                                          getQualityLevel(_activeVehicle.gsm.rssi.value,
                                                          _activeVehicle.gsm.rssiExcellentThreshold.value,
                                                          _activeVehicle.gsm.rssiGoodThreshold.value,
                                                          _activeVehicle.gsm.rssiFairThreshold.value))
                lowest_quality = Math.min(lowest_quality,
                                          getQualityLevel(_activeVehicle.gsm.rscp.value,
                                                          _activeVehicle.gsm.rscpExcellentThreshold,
                                                          _activeVehicle.gsm.rscpGoodThreshold,
                                                          _activeVehicle.gsm.rscpFairThreshold))
                lowest_quality = Math.min(lowest_quality,
                                          getQualityLevel(_activeVehicle.gsm.ecio.value,
                                                          _activeVehicle.gsm.ecioExcellentThreshold,
                                                          _activeVehicle.gsm.ecioGoodThreshold,
                                                          _activeVehicle.gsm.ecioFairThreshold))
            } else if (_activeVehicle.gsm.linkType.value === _4G) {
                lowest_quality = Math.min(lowest_quality,
                                          getQualityLevel(_activeVehicle.gsm.rssi.value,
                                                          _activeVehicle.gsm.rssiExcellentThreshold.value,
                                                          _activeVehicle.gsm.rssiGoodThreshold.value,
                                                          _activeVehicle.gsm.rssiFairThreshold.value))
                lowest_quality = Math.min(lowest_quality,
                                          getQualityLevel(_activeVehicle.gsm.rsrp.value,
                                                          _activeVehicle.gsm.rsrpExcellentThreshold,
                                                          _activeVehicle.gsm.rsrpGoodThreshold,
                                                          _activeVehicle.gsm.rsrpFairThreshold))
                lowest_quality = Math.min(lowest_quality,
                                          getQualityLevel(_activeVehicle.gsm.sinr.value,
                                                          _activeVehicle.gsm.sinrExcellentThreshold,
                                                          _activeVehicle.gsm.sinrGoodThreshold,
                                                          _activeVehicle.gsm.sinrFairThreshold))
                lowest_quality = Math.min(lowest_quality,
                                          getQualityLevel(_activeVehicle.gsm.rsrq.value,
                                                          _activeVehicle.gsm.rsrqExcellentThreshold,
                                                          _activeVehicle.gsm.rsrqGoodThreshold,
                                                          _activeVehicle.gsm.rsrqFairThreshold))
            } else {
                lowest_quality = bad
            }

            return getTextColor(lowest_quality)
        }
        return qgcPal.colorGrey
    }


    function getValueTextColor(value, threshold_excellent, threshold_good, threshold_fair) {
        if(_activeVehicle && !isNaN(value)) {
            return getTextColor(getQualityLevel(value, threshold_excellent, threshold_good, threshold_fair))
        }
        return qgcPal.colorGrey
    }


    function getTextColor(value) {
        if (value == excellent) {
            return qgcPal.colorGreen
        }
        if (value == good) {
            return qgcPal.colorBlue
        }
        if (value == fair) {
            return qgcPal.brandingPurple
        }
        return qgcPal.colorRed
    }


    function getLinkStatusText() {
        if(_activeVehicle) {
            if(_activeVehicle.gsm.linkType.value === 4) {
                return "4G"
            }
            if(_activeVehicle.gsm.linkType.value === 3) {
                return "3G"
            }
            if(_activeVehicle.gsm.linkType.value === 2) {
                return "2G"
            }
            if(_activeVehicle.gsm.linkType.value === 1) {
                return "Unknown"
            }
        }
        return "N/A"
    }


    function getModemText() {
        if(_activeVehicle) {
            if(_activeVehicle.gsm.modemType.value === 0) {
                return "Unkown"
            }
            if(_activeVehicle.gsm.modemType.value === 1) {
                return "HUAWEI E3372"
            }
        }
        return "N/A"
    }


    Component {
        id: gsmInfo

        Rectangle {
            width:  gsmCol.width   + ScreenTools.defaultFontPixelWidth  * 3
            height: gsmCol.height  + ScreenTools.defaultFontPixelHeight * 2
            radius: ScreenTools.defaultFontPixelHeight * 0.5
            color:  qgcPal.window
            border.color:   qgcPal.text

            Column {
                id:                 gsmCol
                spacing:            ScreenTools.defaultFontPixelHeight * 0.5
                width:              Math.max(gsmGrid.width, gsmLabel.width)
                anchors.margins:    ScreenTools.defaultFontPixelHeight
                anchors.centerIn:   parent

                QGCLabel {
                    id:             gsmLabel
                    text:           qsTr("GSM Status")
                    font.family:    ScreenTools.demiboldFontFamily
                    anchors.horizontalCenter: parent.horizontalCenter
                }

                GridLayout {
                    id:                 gsmGrid
                    anchors.margins:    ScreenTools.defaultFontPixelHeight
                    columnSpacing:      ScreenTools.defaultFontPixelWidth
                    columns:            2
                    anchors.horizontalCenter: parent.horizontalCenter

                    QGCLabel { text: qsTr("RSSI:") }
                    QGCLabel {
                        text: (_activeVehicle && !isNaN(_activeVehicle.gsm.rssi.value)) ? (_activeVehicle.gsm.rssi.valueString + " " + _activeVehicle.gsm.rssi.units) : "N/A"
                        color: getValueTextColor(_activeVehicle.gsm.rssi.value,
                                                 _activeVehicle.gsm.rssiExcellentThreshold.value,
                                                 _activeVehicle.gsm.rssiGoodThreshold.value,
                                                 _activeVehicle.gsm.rssiFairThreshold.value)
                    }
                    QGCLabel { text: qsTr("RSRP:") }
                    QGCLabel {
                        text: (_activeVehicle && !isNaN(_activeVehicle.gsm.rsrp.value)) ? (_activeVehicle.gsm.rsrp.valueString + " " + _activeVehicle.gsm.rsrp.units) : "N/A"
                        color: getValueTextColor(_activeVehicle.gsm.rsrp.value,
                                                 _activeVehicle.gsm.rsrpExcellentThreshold,
                                                 _activeVehicle.gsm.rsrpGoodThreshold,
                                                 _activeVehicle.gsm.rsrpFairThreshold)
                    }
                    QGCLabel { text: qsTr("RSCP:") }
                    QGCLabel {
                        text: (_activeVehicle && !isNaN(_activeVehicle.gsm.rscp.value)) ? (_activeVehicle.gsm.rscp.valueString + " " + _activeVehicle.gsm.rscp.units) : "N/A"
                        color: getValueTextColor(_activeVehicle.gsm.rscp.value,
                                                 _activeVehicle.gsm.rscpExcellentThreshold,
                                                 _activeVehicle.gsm.rscpGoodThreshold,
                                                 _activeVehicle.gsm.rscpFairThreshold)
                    }
                    QGCLabel { text: qsTr("SINR:") }
                    QGCLabel {
                        text: (_activeVehicle && !isNaN(_activeVehicle.gsm.sinr.value)) ? (_activeVehicle.gsm.sinr.valueString + " " + _activeVehicle.gsm.sinr.units) : "N/A"
                        color: getValueTextColor(_activeVehicle.gsm.sinr.value,
                                                 _activeVehicle.gsm.sinrExcellentThreshold,
                                                 _activeVehicle.gsm.sinrGoodThreshold,
                                                 _activeVehicle.gsm.sinrFairThreshold)
                    }
                    QGCLabel { text: qsTr("ECIO:") }
                    QGCLabel {
                        text: (_activeVehicle && !isNaN(_activeVehicle.gsm.ecio.value)) ? (_activeVehicle.gsm.ecio.valueString + " " + _activeVehicle.gsm.ecio.units) : "N/A"
                        color: getValueTextColor(_activeVehicle.gsm.ecio.value,
                                                 _activeVehicle.gsm.ecioExcellentThreshold,
                                                 _activeVehicle.gsm.ecioGoodThreshold,
                                                 _activeVehicle.gsm.ecioFairThreshold)
                    }
                    QGCLabel { text: qsTr("RSRQ:") }
                    QGCLabel {
                        text: (_activeVehicle && !isNaN(_activeVehicle.gsm.rsrq.value)) ? (_activeVehicle.gsm.rsrq.valueString + " " + _activeVehicle.gsm.rsrq.units) : "N/A"
                        color: getValueTextColor(_activeVehicle.gsm.rsrq.value,
                                                 _activeVehicle.gsm.rsrqExcellentThreshold,
                                                 _activeVehicle.gsm.rsrqGoodThreshold,
                                                 _activeVehicle.gsm.rsrqFairThreshold)
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
        id:             gsmIndicatorRow
        anchors.top:    parent.top
        anchors.bottom: parent.bottom
        visible:        _activeVehicle ? _activeVehicle.gsm.present.value : false
        QGCLabel {
            text:                   getLinkStatusText()
            font.pointSize:         ScreenTools.largeFontPointSize
            font.bold:              true
            color:                  getMainTextColor()
            anchors.verticalCenter: parent.verticalCenter
        }
    }
    MouseArea {
        anchors.fill:   parent
        onClicked:      mainWindow.showPopUp(gsmInfo, mapToItem(toolBar, x, y).x + (width / 2))
    }
}
