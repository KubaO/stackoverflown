import QtQuick 2.0
import QtQuick.Controls 1.0

ApplicationWindow {
    width: 300; height: 300
    ListView {
        id: view
        width: parent.width
        anchors.top: parent.top
        anchors.bottom: column.top
        model: model1
        spacing: 2
        delegate: Component {
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
                    text: edit // "edit" role of the model, to break the binding loop
                    onTextChanged: model.display = text
                }
            }
        }
    }
    Column {
        id: column;
        anchors.bottom: parent.bottom
        Text { text: "Type";  }
        ComboBox {
            id: box1
            model: model1
            textRole: "display"
            onCurrentTextChanged: generator.generate(currentText)
        }
        Text { text: "Unit"; }
        ComboBox {
            id: box2
            model: model2
            textRole: "display"
        }
    }
}
