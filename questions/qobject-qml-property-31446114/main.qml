import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Window 2.0

Window {
    visible: true
    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        Text {
            text: "Computer price: " + computer.price
        }
        Text {
            text: "CPU price: " + (computer.cpu ? computer.cpu.price : "N/A")
        }
        Button {
            text: "Use CPU 1";
            onClicked: { computer.setCpu(cpu1) }
        }
        Button {
            text: "Use CPU 2";
            onClicked: { computer.setCpu(cpu2) }
        }
        Button {
            text: "Use no CPU";
            onClicked: { computer.setCpu(undefined) }
        }
    }
}
