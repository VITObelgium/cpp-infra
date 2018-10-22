import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.1

Item {
    id: legendView
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
        spacing: 2
        anchors.fill: parent
        delegate: Item {
            implicitWidth: legendItem.width
            height: 20

            RowLayout {
                id: legendItem
                implicitWidth: colorRectangle.width + colorText.width + spacing
                Rectangle {
                    id: colorRectangle
                    width: 20
                    height: 20
                    color: Color
                    border.width: 1
                    border.color: "black"
                }

                Text {
                    id: colorText
                    text: Name
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                }
                spacing: 5
            }
        }
    }
}
