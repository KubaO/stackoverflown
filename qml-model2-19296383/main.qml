import QtQuick 2.0
import QtQml.Models 2.1
import QtQuick.Controls 1.0

ApplicationWindow {
    width: 300; height: 300
    Row {
        width: parent.width
        anchors.top: parent.top
        anchors.bottom: column.top
        Component {
            id: commonDelegate
            Rectangle {
                width: view.width
                implicitHeight: editor.implicitHeight + 10
                color: "transparent"
                border.color: "red"
                border.width: 2
                radius: 5
                TextInput {
                    id: editor
                    anchors.margins: 1.5 * parent.border.width
                    anchors.fill: parent
                    text: edit.name // "edit" role of the model, to break the binding loop
                    onTextChanged: {
                        display.name = text;
                        model.display = display
                    }
                }
            }
        }
        ListView {
            id: view
            width: parent.width / 2
            height: parent.height
            model: DelegateModel {
                id: delegateModel1
                model: model1
                delegate: commonDelegate
            }
            spacing: 2
        }
        ListView {
            width: parent.width / 2
            height: parent.height
            model: DelegateModel {
                model: model1
                delegate: commonDelegate
            }
            spacing: 2
        }
    }
    Column {
        id: column;
        anchors.bottom: parent.bottom
        Row {
            Button {
                text: "Add Page";
                onClicked: model1.insertRows(delegateModel1.count, 1)
            }
            Button {
                text: "Remove Page";
                onClicked: model1.removeRows(pageNo.value - 1, 1)
            }
            SpinBox {
                id: pageNo
                minimumValue: 1
                maximumValue: delegateModel1.count;
            }
        }
    }
}
