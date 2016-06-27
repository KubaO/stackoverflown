#!/usr/bin/env python
#https://github.com/KubaO/stackoverflown/tree/master/questions/interval-timer-38036583
# -*- coding: utf-8 -*-
import sys
from PyQt4 import QtCore, QtGui

if __name__ == "__main__":
    running = False
    app = QtGui.QApplication(sys.argv)
    w = QtGui.QWidget()
    layout = QtGui.QVBoxLayout(w)
    label = QtGui.QLabel()
    button = QtGui.QPushButton('Start')
    timer = QtCore.QElapsedTimer()
    updateTimer = QtCore.QTimer()

    layout.addWidget(label)
    layout.addWidget(button)

    def onTimeout():
        label.setText('Elapsed: {0}ms'.format(timer.elapsed()))

    def onClicked():
        global running
        if running:
            onTimeout()
            updateTimer.stop()
            button.setText('Start')
        else:
            timer.start()
            updateTimer.start()
            button.setText('Stop')
        running = not running

    updateTimer.setInterval(1000/25) # ~25fps update rate
    updateTimer.timeout.connect(onTimeout)
    button.clicked.connect(onClicked)

    w.show()
    sys.exit(app.exec_())
