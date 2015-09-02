#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from PySide import QtCore, QtGui

class FlatProxyModel(QtGui.QAbstractProxyModel):
    @QtCore.Slot(QtCore.QModelIndex, QtCore.QModelIndex)
    def sourceDataChanged(self, topLeft, bottomRight):
        self.dataChanged.emit(self.mapFromSource(topLeft), \
                              self.mapFromSource(bottomRight))
    def buildMap(self, model, parent = QtCore.QModelIndex(), row = 0):
        if row == 0:
            self.m_rowMap = {}
            self.m_indexMap = {}
        rows = model.rowCount(parent)
        for r in range(rows):
            index = model.index(r, 0, parent)
            print('row', row, 'item', model.data(index))
            self.m_rowMap[index] = row
            self.m_indexMap[row] = index
            row = row + 1
            if model.hasChildren(index):
                row = self.buildMap(model, index, row)
        return row
    def setSourceModel(self, model):
        QtGui.QAbstractProxyModel.setSourceModel(self, model)
        self.buildMap(model)
        print(flush = True)
        model.dataChanged.connect(self.sourceDataChanged)
    def mapFromSource(self, index):
        if index not in self.m_rowMap: return QtCore.QModelIndex()
        #print('mapping to row', self.m_rowMap[index], flush = True)
        return self.createIndex(self.m_rowMap[index], index.column())
    def mapToSource(self, index):
        if not index.isValid() or index.row() not in self.m_indexMap:
            return QtCore.QModelIndex()
        #print('mapping from row', index.row(), flush = True)
        return self.m_indexMap[index.row()]
    def columnCount(self, parent):
        return QtGui.QAbstractProxyModel.sourceModel(self)\
               .columnCount(self.mapToSource(parent))
    def rowCount(self, parent):
        #print('rows:', len(self.m_rowMap), flush=True)
        return len(self.m_rowMap) if not parent.isValid() else 0
    def index(self, row, column, parent):
        #print('index for:', row, column, flush=True)
        if parent.isValid(): return QtCore.QModelIndex()
        return self.createIndex(row, column)
    def parent(self, index):
        return QtCore.QModelIndex()
    def __init__(self, parent = None):
        super(FlatProxyModel, self).__init__(parent)

if __name__ == "__main__":
    app = QtGui.QApplication(sys.argv)

    model = QtGui.QStandardItemModel()
    names = ['Foo', 'Bar', 'Baz']
    for first in names:
        row = QtGui.QStandardItem(first)
        for second in names:
            row.appendRow(QtGui.QStandardItem(first+second))
        model.appendRow(row)

    proxy = FlatProxyModel()
    proxy.setSourceModel(model)

    nestedProxy = FlatProxyModel()
    nestedProxy.setSourceModel(proxy)

    w = QtGui.QWidget()
    layout = QtGui.QHBoxLayout(w)
    view = QtGui.QTreeView()
    view.setModel(model)
    view.expandAll()
    view.header().hide()
    layout.addWidget(view)
    view = QtGui.QListView()
    view.setModel(proxy)
    layout.addWidget(view)
    view = QtGui.QListView()
    view.setModel(nestedProxy)
    layout.addWidget(view)
    w.show()

    sys.exit(app.exec_())
