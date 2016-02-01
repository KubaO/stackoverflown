// https://github.com/KubaO/stackoverflown/tree/master/questions/css-selector-35129103
#include <QtWidgets>
#include <private/qcssparser_p.h>

using namespace QCss;

// FROM src/widgets/styles/qstylesheetstyle.cpp

#define OBJECT_PTR(node) (static_cast<QObject*>((node).ptr))

static inline QObject *parentObject(const QObject *obj)
{
   if (qobject_cast<const QLabel *>(obj) && qstrcmp(obj->metaObject()->className(), "QTipLabel") == 0) {
      QObject *p = qvariant_cast<QObject *>(obj->property("_q_stylesheet_parent"));
      if (p)
         return p;
   }
   return obj->parent();
}

class QObjectStyleSelector : public StyleSelector
{
public:
   QObjectStyleSelector() { }

   QStringList nodeNames(NodePtr node) const Q_DECL_OVERRIDE
   {
      if (isNullNode(node))
         return QStringList();
      const QMetaObject *metaObject = OBJECT_PTR(node)->metaObject();
#ifndef QT_NO_TOOLTIP
      if (qstrcmp(metaObject->className(), "QTipLabel") == 0)
         return QStringList(QLatin1String("QToolTip"));
#endif
      QStringList result;
      do {
         result += QString::fromLatin1(metaObject->className()).replace(QLatin1Char(':'), QLatin1Char('-'));
         metaObject = metaObject->superClass();
      } while (metaObject != 0);
      return result;
   }
   QString attribute(NodePtr node, const QString& name) const Q_DECL_OVERRIDE
   {
      if (isNullNode(node))
         return QString();

      auto obj = OBJECT_PTR(node);
      auto value = obj->property(name.toLatin1());
      if (!value.isValid()) {
         if (name == QLatin1String("class")) {
            auto className = QString::fromLatin1(obj->metaObject()->className());
            if (className.contains(QLatin1Char(':')))
               className.replace(QLatin1Char(':'), QLatin1Char('-'));
            return className;
         }
      }
      if(value.type() == QVariant::StringList || value.type() == QVariant::List)
         return value.toStringList().join(QLatin1Char(' '));
      else
         return value.toString();
   }
   bool nodeNameEquals(NodePtr node, const QString& nodeName) const Q_DECL_OVERRIDE
   {
      if (isNullNode(node))
         return false;
      auto metaObject = OBJECT_PTR(node)->metaObject();
#ifndef QT_NO_TOOLTIP
      if (qstrcmp(metaObject->className(), "QTipLabel") == 0)
         return nodeName == QLatin1String("QToolTip");
#endif
      do {
         const ushort *uc = (const ushort *)nodeName.constData();
         const ushort *e = uc + nodeName.length();
         const uchar *c = (const uchar *)metaObject->className();
         while (*c && uc != e && (*uc == *c || (*c == ':' && *uc == '-'))) {
            ++uc;
            ++c;
         }
         if (uc == e && !*c)
            return true;
         metaObject = metaObject->superClass();
      } while (metaObject != 0);
      return false;
   }
   bool hasAttributes(NodePtr) const Q_DECL_OVERRIDE
   { return true; }
   QStringList nodeIds(NodePtr node) const Q_DECL_OVERRIDE
   { return isNullNode(node) ? QStringList() : QStringList(OBJECT_PTR(node)->objectName()); }
   bool isNullNode(NodePtr node) const Q_DECL_OVERRIDE
   { return node.ptr == 0; }
   NodePtr parentNode(NodePtr node) const Q_DECL_OVERRIDE
   { NodePtr n; n.ptr = isNullNode(node) ? 0 : parentObject(OBJECT_PTR(node)); return n; }
   NodePtr previousSiblingNode(NodePtr) const Q_DECL_OVERRIDE
   { NodePtr n; n.ptr = 0; return n; }
   NodePtr duplicateNode(NodePtr node) const Q_DECL_OVERRIDE
   { return node; }
   void freeNode(NodePtr) const Q_DECL_OVERRIDE
   { }
};

// END FROM

QWidgetList select(const QString & selector) {
   QWidgetList result;
   Parser parser(selector + "{}");
   QObjectStyleSelector oSel;
   oSel.styleSheets.append(StyleSheet());
   if (!parser.parse(&oSel.styleSheets[0]))
      return result;

   for (auto top : qApp->topLevelWidgets()) {
      auto widgets = top->findChildren<QWidget*>();
      widgets << top;
      for (auto widget : widgets) {
         StyleSelector::NodePtr n;
         n.ptr = widget;
         auto rules = oSel.styleRulesForNode(n);
         if (!rules.isEmpty()) result << widget;
      }
   }
   return result;
}

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QDialog dialog;
   QPushButton button{"OK", &dialog};
   button.setObjectName("okButton");
   button.setStyleSheet("color: red");

   auto dialog_l = QWidgetList() << &dialog;
   auto button_l = QWidgetList() << &button;
   auto all_l = button_l + dialog_l;
   Q_ASSERT(select("QPushButton#okButton") == button_l);
   Q_ASSERT(select("QDialog QPushButton") == button_l);
   Q_ASSERT(select("QDialog") == dialog_l);
   Q_ASSERT(select("QDialog, QPushButton") == all_l);
}

