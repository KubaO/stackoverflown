// https://github.com/KubaO/stackoverflown/tree/master/questions/scroll-widget-array-32459693
#include <QtWidgets>

// Taken from http://doc.qt.io/qt-5/qtwidgets-layouts-flowlayout-example.html
class FlowLayout : public QLayout
{
   QList<QLayoutItem *> itemList;
   int m_hSpace;
   int m_vSpace;
   int doLayout(const QRect &rect, bool testOnly) const;
   int smartSpacing(QStyle::PixelMetric pm) const;
public:
   explicit FlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1)
      : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
   {
      setContentsMargins(margin, margin, margin, margin);
   }
   explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1)
      : m_hSpace(hSpacing), m_vSpace(vSpacing)
   {
      setContentsMargins(margin, margin, margin, margin);
   }
   ~FlowLayout() { while (auto item = takeAt(0)) delete item;   }

   void addItem(QLayoutItem *item) Q_DECL_OVERRIDE { itemList.append(item); }
   int horizontalSpacing() const {
      return (m_hSpace >= 0) ?
               m_hSpace : smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
   }
   int verticalSpacing() const {
      if (m_vSpace >= 0) {
         return m_vSpace;
      } else {
         return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
      }
   }
   Qt::Orientations expandingDirections() const Q_DECL_OVERRIDE { return Qt::Horizontal; }
   bool hasHeightForWidth() const Q_DECL_OVERRIDE { return true; }
   int heightForWidth(int width) const Q_DECL_OVERRIDE {
      return doLayout(QRect(0, 0, width, 0), true);
   }
   int count() const Q_DECL_OVERRIDE { return itemList.size(); }
   QLayoutItem *itemAt(int index) const Q_DECL_OVERRIDE {
      return itemList.value(index);
   }
   QSize minimumSize() const Q_DECL_OVERRIDE {
      QSize size;
      for (auto item : itemList) size = size.expandedTo(item->minimumSize());
      size += QSize(2*margin(), 2*margin());
      qDebug() << __FUNCTION__ << size;
      return size;
   }
   void setGeometry(const QRect &rect) Q_DECL_OVERRIDE {
      QLayout::setGeometry(rect);
      doLayout(rect, false);
   }
   QSize sizeHint() const Q_DECL_OVERRIDE { return minimumSize(); }
   QLayoutItem *takeAt(int index) Q_DECL_OVERRIDE {
      return (index >= 0 && index < itemList.size()) ? itemList.takeAt(index) : 0;
   }
};

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
   int left, top, right, bottom;
   getContentsMargins(&left, &top, &right, &bottom);
   QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
   int x = effectiveRect.x();
   int y = effectiveRect.y();
   int lineHeight = 0;

   while (true) {
      // Row
      QSize min { 0, 0 }, max;
   }

   for (auto item : itemList) {
      auto wid = item->widget();
      int spaceX = horizontalSpacing();
      if (spaceX == -1)
         spaceX = wid->style()->layoutSpacing(
                  QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
      int spaceY = verticalSpacing();
      if (spaceY == -1)
         spaceY = wid->style()->layoutSpacing(
                  QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
      int nextX = x + item->sizeHint().width() + spaceX;
      if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
         x = effectiveRect.x();
         y = y + lineHeight + spaceY;
         nextX = x + item->sizeHint().width() + spaceX;
         lineHeight = 0;
      }

      if (!testOnly) item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

      x = nextX;
      lineHeight = qMax(lineHeight, item->sizeHint().height());
   }
   return y + lineHeight - rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
   auto parent = this->parent();
   if (!parent) {
      return -1;
   } else if (parent->isWidgetType()) {
      auto pw = static_cast<QWidget *>(parent);
      return pw->style()->pixelMetric(pm, 0, pw);
   } else {
      return static_cast<QLayout *>(parent)->spacing();
   }
}

class ParamBlock : public QWidget {
   Q_OBJECT
   Q_PROPERTY(int min READ min WRITE setMin)
   Q_PROPERTY(int max READ max WRITE setMax)
   Q_PROPERTY(int value READ value WRITE setValue USER true)
   QHBoxLayout top { this };
   QGroupBox box;
   QHBoxLayout layout { &box };
   QSlider slider;
   QLabel minLabel, maxLabel;
   QLineEdit edit;
   void on_edit_textEdited(const QString & text) {
      auto value = text.toInt();
      auto newValue = qBound(slider.minimum(), value, slider.maximum());
      slider.setValue(newValue);
   }
   void on_slider_valueChanged(int value) {
      edit.setText(QString::number(value));
   }
   bool event(QEvent * ev) Q_DECL_OVERRIDE {
      if (ev->type() == QEvent::FontChange) fontChangeEvent();
      return QWidget::event(ev);
   }
   void fontChangeEvent() {
      QFontMetrics fm { font() };
      edit.setFixedWidth(edit.minimumSizeHint().width() +
                         fm.width(QString(edit.maxLength()-1, QLatin1Char('0'))));
   }
public:
   ParamBlock(QWidget * parent = 0) : QWidget(parent) {
      top.addWidget(&box);
      top.setMargin(0);
      layout.addWidget(&minLabel);
      layout.addWidget(&slider);
      layout.addWidget(&maxLabel);
      layout.addWidget(&edit);
      slider.setOrientation(Qt::Horizontal);
      slider.setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
      edit.setMaxLength(2);
      connect(&edit, &QLineEdit::textEdited, this, &ParamBlock::on_edit_textEdited);
      connect(&slider, &QSlider::valueChanged, this, &ParamBlock::on_slider_valueChanged);
      fontChangeEvent();
      setMin(min());
      setMax(max());
   }
   int min() const { return slider.minimum(); }
   int max() const { return slider.maximum(); }
   int value() const { return slider.value(); }
   void setMin(int min) {
      minLabel.setNum(min);
      slider.setMinimum(min);
   }
   void setMax(int max) {
      maxLabel.setNum(max);
      slider.setMaximum(max);
   }
   void setValue(int val) {
      slider.setValue(val);
   }
   void setTitle(const QString & title) {
      box.setTitle(title);
   }
};

int main(int argc, char *argv[])
{
   QApplication app{argc, argv};
   QScrollArea scrollArea;
   QWidget box;
   QGridLayout layout{&box};
   layout.setSizeConstraint(QLayout::SetMinimumSize);
   scrollArea.setWidgetResizable(true);

   int n = 1, row = 0, column = 0;
   for(int k=0; k<20; ++k)
   {
      for(int i=0; i<6;++i)
      {
         auto widget = new ParamBlock;
         widget->setTitle(QString("Widget #%1").arg(n++));
         widget->setValue(25);
         widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
         layout.addWidget(widget, row, column);
         ++row;
         if (row>14) {
            row = 0;
            ++column;
         }
      }
   }
   if (1) {
      scrollArea.setWidget(&box);
      scrollArea.show();
   } else {
      box.show();
   }
   return app.exec();
}

#include "main.moc"
