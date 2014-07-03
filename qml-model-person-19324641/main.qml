import QtQuick 2.0
import QtQml.Models 2.1
import QtQuick.Controls 1.0

ApplicationWindow {
    width: 300; height: 300
    Row {
        width: parent.width
        anchors.top: parent.top
        anchors.bottom: row2.top
        Component {
            id: commonDelegate
            Rectangle {
                width: view.width
                implicitHeight: editor.implicitHeight + 10
                border.color: "red"
                border.width: 2
                radius: 5
                TextInput {
                    id: editor
                    anchors.margins: 1.5 * parent.border.width
                    anchors.fill: parent
                    text: edit.name // "edit" role of the model, to break the binding loop
                    onTextChanged: {
                        display.name = text; // set the user of the data object
                        // equivalent to: display.name = text
                    }
                }
                Menu {
                  id: myContextMenu
                  MenuItem { text: "Randomize"; onTriggered: display.setRandomName() }
                  MenuItem { text: "Remove"; onTriggered: model1.removeRows(index, 1) }
                }
                MouseArea {
                  id: longPressArea
                  anchors.fill: parent
                  acceptedButtons: Qt.RightButton
                  onClicked: myContextMenu.popup()
                }
            }
        }
        spacing: 2
        ListView {
            id: view
            width: (parent.width - parent.spacing)/2
            height: parent.height
            model: DelegateModel {
                id: delegateModel1
                model: model1
                delegate: commonDelegate
            }
            spacing: 2
        }
        ListView {
            width: (parent.width - parent.spacing)/2
            height: parent.height
            model: DelegateModel {
                model: model1
                delegate: commonDelegate
            }
            spacing: 2
        }
    }
    Row {
        id: row2
        anchors.bottom: parent.bottom
        Button {
            text: "Add Page";
            onClicked: model1.insertRows(delegateModel1.count, 1)
        }

    }
}
