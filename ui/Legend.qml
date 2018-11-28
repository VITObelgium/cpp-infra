import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.1

Item {
    id: legendView
    property double scale: 1.0
    property alias model: listView.model
    property alias count: listView.count
    implicitWidth: listView.implicitWidth
    implicitHeight: listView.implicitHeight

    Rectangle
    {
        id: background
        anchors.fill: parent
        color: "white"
        border.width: 1
        border.color: "black"
    }

    ListView {
        id: listView
        implicitWidth: contentItem.childrenRect.width + anchors.margins * 2
        implicitHeight: contentItem.childrenRect.height + anchors.margins * 2
        anchors.margins: 5
        orientation: ListView.Vertical
        spacing: 2 * legendView.scale
        anchors.fill: parent
        delegate: Item {
            implicitWidth: legendItem.width
            height: 20 * legendView.scale

            RowLayout {
                id: legendItem
                implicitWidth: 20 * legendView.scale + colorText.width + spacing
                Rectangle {
                    id: colorRectangle
                    Layout.preferredWidth: 20 * legendView.scale
                    Layout.preferredHeight: 20 * legendView.scale
                    //anchors.bottom: parent.bottom
                    //anchors.left: parent.left
                    width: height
                    color: Color
                    border.color: "black"
                }

                Text {
                    id: colorText
                    text: Name
                    //anchors.left: colorRectangle.right
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                    font.pointSize: 10.0 * legendView.scale
                }

                spacing: 2 * legendView.scale
            }
        }
    }
}
