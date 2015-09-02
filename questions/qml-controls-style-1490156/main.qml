    import QtQuick 2.1
    import QtQuick.Layouts 1.0
    import QtQuick.Controls 1.0

    ApplicationWindow {
        toolBar: ToolBar {
            id: toolbar
            Component.onCompleted: toolbar.data[0].item.children = [newRectangle];
            property Item _newRectangle: Rectangle {
                // The rectangle within the ToolBarStyle's panel
                // Gleaned from:
                // http://qt.gitorious.org/qt/qtquickcontrols/source/
                //   c304d741a27b5822a35d1fb83f8f5e65719907ce:src/styles/Base/ToolBarStyle.qml
                id: newRectangle
                anchors.fill: parent
                gradient: Gradient{
                    GradientStop{color: "#a00" ; position: 0}
                    GradientStop{color: "#aaa" ; position: 1}
                }
                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: "#999"
                }
            }
            RowLayout {
                ToolButton { iconSource: "image://images/img1" }
                ToolButton { iconSource: "image://images/img2" }
            }
        }
    }
