#ifndef GUI_H
#define GUI_H

#include <QTimer>
#include <QHeaderView>
#include <QCheckBox>
#include <QTableView>
#include <QAbstractTableModel>
#include <QTableView>

class ConsoleWindowView : public QTableView
{
   Q_OBJECT
   Q_PROPERTY(bool scrollToNewest READ scrollToNewest WRITE setScrollToNewest)
   bool m_scrollToNewest;
   Q_SLOT void modelRowsInserted(const QModelIndex &, int start, int end) {
      Q_UNUSED(end)
      if (model() && m_scrollToNewest) scrollTo(model()->index(start, 0));
   }
public:
   ConsoleWindowView(QWidget *parent = 0) : QTableView(parent), m_scrollToNewest(true)
   {}
   void setModel(QAbstractItemModel *TheModel) Q_DECL_OVERRIDE
   {
      QTableView::setModel(TheModel);
      connect(model(), &QAbstractItemModel::rowsInserted, this, &ConsoleWindowView::modelRowsInserted);
   }
   Q_SLOT void setScrollToNewest(bool s) { m_scrollToNewest = s; }
   bool scrollToNewest() const { return m_scrollToNewest; }
};

class ConsoleWindowModel : public QAbstractTableModel
{
   Q_OBJECT
   unsigned int m_rowCount;
   QTimer m_controllerTimer;
   Q_SLOT void updateController()
   {
      beginInsertRows(QModelIndex(), m_rowCount, m_rowCount);
      m_rowCount++;
      endInsertRows();
   }
public:
   ConsoleWindowModel(QObject *parent = 0)
      : QAbstractTableModel(parent)
      , m_rowCount(0) {
      connect(&m_controllerTimer, &QTimer::timeout, this, &ConsoleWindowModel::updateController);
      m_controllerTimer.start(2000);
   }
   int rowCount(const QModelIndex &) const Q_DECL_OVERRIDE {
      return m_rowCount;
   }
   int columnCount(const QModelIndex &) const Q_DECL_OVERRIDE {
      return 2;
   }
   QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE {
      if (role == Qt::DisplayRole) {
         return QString("Row%1, Column%2")
               .arg(index.row() + 1)
               .arg(index.column() +1);
      }
      return QVariant();
   }
   QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE {
      if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
         switch(section) {
         case 0:
            return "Stamp";
            break;
         case 1:
            return "Text";
            break;
         default:
            return QString("Column %1").arg(section + 1);
            break;
         }
      }
      else if (orientation == Qt::Vertical && role == Qt::DisplayRole)
         return QString("%1").arg(section + 1);
      return QVariant();
   }
};

#include "ui_Gui.h"

class Gui : public QMainWindow
{
   Q_OBJECT
   Ui::Gui ui;
   ConsoleWindowModel consoleWindowModel;
   void connectConsoleWindow() {
      auto view = ui.ConsoleOutputTable;
      view->setModel(&consoleWindowModel);
      for (int c = 1; c < view->horizontalHeader()->count(); ++c)
         view->horizontalHeader()->setSectionResizeMode(c, QHeaderView::Stretch);

      connect(ui.AutoScroll, &QCheckBox::toggled, view, &ConsoleWindowView::setScrollToNewest);
   }

public:
   Gui(QWidget *parent = 0) : QMainWindow(parent) {
      ui.setupUi(this);
      ui.actionExit->setShortcuts(QKeySequence::Quit);
      ui.actionExit->setStatusTip(tr("Exit the application"));
      connect(ui.actionExit, &QAction::triggered, this, &QWidget::close);
      connectConsoleWindow();
   }
};

#endif // GUI_H
