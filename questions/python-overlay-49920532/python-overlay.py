# https://github.com/KubaO/stackoverflown/tree/master/questions/python-overlay-49920532
from PyQt5 import Qt
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
import sys

class Overlay(QWidget):
    def __init__(self, parent = None):
        QWidget.__init__(self, parent)
        self.setAttribute(Qt.WA_NoSystemBackground)
        self.setAttribute(Qt.WA_TransparentForMouseEvents)
        
    def paintEvent(self, event) {
        QPainter(self).fillrect(self.rect(), QColor(80, 80, 255, 128))

class Filter(QObject):
    def __init__(self, parent = None):
        QObject.__init__(self, parent)
        self.m_overlay = None
        self.m_overlayOn = None
    
    def eventFilter(self, w, event):
        if w.isWidgetType():
            if event.type() == QEvent.MouseButtonPress:
                if not m_overlay:
                    m_overlay = Overlay(w.parentWidget());
                    m_overlay.setGeometry(w.geometry());
                    m_overlayOn = w;
                    m_overlay.show();
            elif event.type() == QEvent.Resize:
                if m_overlay and m_overlayOn is w:
                    m_overlay.setGeometry(w.geometry());
        return False
        
if __name__ == "__main__":
    app = QApplication(sys.argv)
    filter = Filter()
    window = QWidget()
    layout = QHBoxLayout(window);
    for text in ["Foo", "Bar", "Baz"]:
        label = QLabel(text)
        label.installEventFilter(filter)
        layout.addWidget(label)
    window.setMinimumSize(300, 250)
    window.show()
    app.exec_()
