import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11
import QtPositioning 5.5
import QtLocation 5.6

import inf.ui.location 1.0

import "qrc:/qmlcontrols"

Item {
    id: mapView
    focus: true

    Rectangle
    {
        id: background
        anchors.fill: parent
        color: "darkgray"
        z: -10
    }

    Plugin {
        id: mapPlugin
        name: "esri" // "mapboxgl", "esri", ...
        // Disabled: required ssl libraries
        // PluginParameter {
        //      name: "osm.mapping.highdpi_tiles"
        //      value: true
        // }
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_Plus) {
            map.zoomLevel += 0.5
            event.accepted = true;
        } else if (event.key == Qt.Key_Minus) {
            map.zoomLevel -= 0.5
            event.accepted = true;
        }
    }

    ComboBox {
        id: providerCombo
        editable: false
        width: 250
        z: 10
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 5
        model: ListModel {
            id: typesModel
        }

        onActivated: {
            bgmap.setActiveMapType(currentText)
        }

        Component.onCompleted: {
            typesModel.append({text: "None"})
            for (var i = 0; i < bgmap.supportedMapTypes.length; i++) {
                var mapType = bgmap.supportedMapTypes[i]
                typesModel.append({text: mapType.name})
            }

            currentIndex = 1
        }
    }

    Button {
        id: plus
        z: 10
        width: 30
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 5
        text: "+"
        onClicked: {
            map.zoomLevel += 0.5
        }
    }

    Button {
        id: minus
        z: 10
        width: 30
        anchors.right: parent.right
        anchors.top: plus.bottom
        anchors.margins: 5
        text: "-"
        onClicked: {
            map.zoomLevel -= 0.5
        }
    }

    Slider {
        id: opacitySlider
        z: 10
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        value: 0.7
    }

    Item {
        id: textInfoItem
        width: childrenRect.width
        height: childrenRect.height
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 5
        visible: false
        z: 1

        Rectangle {
            color: "darkslategray"
            width: childrenRect.width
            height: childrenRect.height
            radius: 2

            Text {
                id: cursorInfoText
                padding: 2
                leftPadding: 4
                rightPadding: 4
                color: "white"
                font.pointSize: 10
            }
        }
    }

    Map {
        z: -1
        id: map
        objectName: "map"
        color: 'transparent'
        gesture.enabled: true
        anchors.fill: parent
        signal mouseMoved(var coordinate)

        plugin: Plugin { name: "itemsoverlay" }

        MapQuickItem {
            id: raster
            opacity: opacitySlider.value
            objectName: "raster"
            sourceItem: PixmapImage {
                objectName: "rasterimage"
                id: rasterimage
                z: 0
                smooth: false
            }
            anchorPoint: "0,0"
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true

            onPositionChanged : {
                if (valueprovider) {
                    cursorInfoText.text = valueprovider.rasterValueString(map.toCoordinate(Qt.point(mouse.x, mouse.y)))
                    textInfoItem.visible = cursorInfoText.text.length > 0
                }
            }
        }

        MapItemView {
            id: pointSources
            objectName: "pointSources"
            model: pointsourcemodel
            delegate: MapQuickItem {
                    
                sourceItem: Rectangle {
                    z: 1
                    id: rectangle
                    width: 14
                    height: 14
                    color: Color
                    border.width: 2
                    border.color: "white"
                    smooth: true
                    radius: 7

                    ToolTip.text: "<b> " + Name + "</b> (" + Value + " µg/m³)"
                    ToolTip.visible: tooltipArea.containsMouse

                    MouseArea {
                        id: tooltipArea
                        hoverEnabled: true
                        anchors.fill: parent
                    }
                }
                
                coordinate: QtPositioning.coordinate(Latitude, Longitude)
                anchorPoint: Qt.point(sourceItem.width / 2, sourceItem.height / 2)
            }
        }
    }

    RowLayout {
        property bool show: true
        id: legend
        objectName: "legend"
        x: 5
        y: 5
        z: 1
        height: parent.height - 10
        visible: legenditem.count > 0 && show

        Legend {
            opacity:0.8
            id: legenditem
            objectName: "legenditem"
            model: legendmodel

            DragHandler {
                id: handler
                target: legend
                acceptedButtons: Qt.RightButton
            }
        }
    }

    Map {
        z: -2
        id: bgmap
        objectName: "bgmap"
        gesture.enabled: false
        plugin: mapPlugin
        anchors.fill: parent
        visible: true
        copyrightsVisible: false

        center: map.center
        color: 'transparent'
        minimumFieldOfView: map.minimumFieldOfView
        maximumFieldOfView: map.maximumFieldOfView
        minimumTilt: map.minimumTilt
        maximumTilt: map.maximumTilt
        minimumZoomLevel: map.minimumZoomLevel
        maximumZoomLevel: map.maximumZoomLevel
        zoomLevel: map.zoomLevel
        tilt: map.tilt;
        bearing: map.bearing
        fieldOfView: map.fieldOfView

        function setActiveMapType(typeName) {
            if (typeName == "None") {
                bgmap.visible = false
                return
            }

            bgmap.visible = true
            for (var i = 0; i < bgmap.supportedMapTypes.length; i++) {
                var mapType = bgmap.supportedMapTypes[i]
                if (mapType.name == typeName) {
                    bgmap.activeMapType = mapType
                }
            }
        }

        function getMapTypes() {
            var mapTypeNames = []

            for (var i = 0; i < bgmap.supportedMapTypes.length; ++i) {
                var mapType = bgmap.supportedMapTypes[i]
                mapTypeNames.push(mapType.name)
            }
            return mapTypeNames
        }
    }
}
