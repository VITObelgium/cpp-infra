import QtQuick 2.11
import QtQuick.Controls 2.3
import QtPositioning 5.5
import QtLocation 5.6

Item {
    id: mapView
    focus: true

    //Connections {
    //    target: context
    //}

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
            property alias source: rasterimage.source

            id: raster
            opacity: opacitySlider.value
            visible: false
            objectName: "raster"
            sourceItem: Image {
                id: rasterimage
                z: 0
                asynchronous: true
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
