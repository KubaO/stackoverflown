#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from PyQt5.QtCore import QRect, QRectF, QSizeF, QPointF, Qt
from PyQt5.QtGui import QPainter, QPicture, QFont, QColor
from PyQt5.QtWidgets import QApplication, QLabel

def drawNode(painter, angle, radius, text):
    size = 32767.0;
    painter.save();
    painter.rotate(-angle);
    painter.translate(radius, 0);
    painter.drawText(QRectF(0, -size/2.0, size, size), Qt.AlignVCenter, text);
    painter.restore();

if __name__ == "__main__":
    app = QApplication(sys.argv)

    pic = QPicture()
    pic.setBoundingRect(QRect(-100, -100, 200, 200))
    p = QPainter(pic)

    p.drawEllipse(0, 0, 3, 3)
    p.setFont(QFont("Helvetica", 25))
    for angle in range(0, 359, 30):
        drawNode(p, angle, 50, str(angle))
    p.end()

    l = QLabel()
    l.setPicture(pic);
    l.show();

    sys.exit(app.exec_())
