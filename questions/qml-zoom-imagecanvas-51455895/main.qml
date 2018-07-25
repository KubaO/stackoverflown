import QtQuick 2.10
import QtQuick.Controls 2.3
import App 1.0

ApplicationWindow {
    id: window
    width: 600
    height: 600
    visible: true
    title: "T=" + (canvas.renderTime*1E3).toFixed(1) + "ms t=" + canvas.targetRect + " s=" + canvas.sourceRect

    ImageCanvas {
        id: canvas
        image: sampleImage
        anchors.fill: parent
        offset: Qt.point(xOffsetSlider.value, yOffsetSlider.value)
        zoom: Math.pow(Math.SQRT2, zoomSpinBox.value)
        rectDraw: rectDrawCheckBox.checked
    }

    SpinBox {
        id: zoomSpinBox
        anchors.bottom: xOffsetSlider.top
        from: -10
        to: 20
    }

    CheckBox {
        id: rectDrawCheckBox
        anchors.left: zoomSpinBox.right
        anchors.bottom: xOffsetSlider.top
        text: "rectDraw"
        checked: true
    }

    Slider {
        id: xOffsetSlider
        anchors.bottom: parent.bottom
        width: parent.width - height
        from: 0
        to: canvas.imageRect.width

        ToolTip {
            id: xOffsetToolTip
            parent: xOffsetSlider.handle
            visible: true
            text: xOffsetSlider.value.toFixed(1)

            Binding {
                target: xOffsetToolTip
                property: "visible"
                value: !yOffsetToolTip.visible
            }
        }
    }

    Slider {
        id: yOffsetSlider
        anchors.right: parent.right
        height: parent.height - width
        orientation: Qt.Vertical
        from: canvas.imageRect.height
        to: 0

        ToolTip {
            id: yOffsetToolTip
            parent: yOffsetSlider.handle
            text: yOffsetSlider.value.toFixed(1)

            Binding {
                target: yOffsetToolTip
                property: "visible"
                value: !xOffsetToolTip.visible
            }
        }
    }
}
