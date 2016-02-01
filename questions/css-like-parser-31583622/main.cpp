// https://github.com/KubaO/stackoverflown/tree/master/questions/css-like-parser-31583622
#include <QtGui>
#include <private/qcssparser_p.h>

extern const char pss[];

QDebug operator<<(QDebug dbg, const QCss::AttributeSelector & sel) {
   QDebugStateSaver saver(dbg);
   dbg.noquote().nospace() << "\"" << sel.name << "\"";
   switch (sel.valueMatchCriterium) {
   case QCss::AttributeSelector::MatchEqual:
      dbg << "="; break;
   case QCss::AttributeSelector::MatchContains:
      dbg << "~="; break;
   case QCss::AttributeSelector::MatchBeginsWith:
      dbg << "^="; break;
   case QCss::AttributeSelector::NoMatch:
      break;
   }
   if (sel.valueMatchCriterium != QCss::AttributeSelector::NoMatch && !sel.value.isEmpty())
      dbg << "\"" << sel.value << "\"";
   return dbg;
}

QDebug operator<<(QDebug dbg, const QCss::BasicSelector & sel) {
   QDebugStateSaver saver(dbg);
   dbg.noquote().nospace() << "BasicSelector";
   if (!sel.elementName.isEmpty())
      dbg << " #" << sel.elementName;
   for (auto & id : sel.ids)
      dbg << " id:" << id;
   for (auto & aSel : sel.attributeSelectors)
      dbg << " " << aSel;
   return dbg;
}

QDebug operator<<(QDebug dbg, const QCss::Declaration & decl) {
   QDebugStateSaver saver(dbg);
   dbg.noquote().nospace() << "Declaration";
   dbg << " \"" << decl.d->property << "\" = ";
   bool first = true;
   for (auto value : decl.d->values) {
      if (!first) dbg << ", ";
      dbg << "\'" << value.toString() << "\'";
      first = false;
   }
   if (decl.d->property == "fillColor")
      dbg << " % " << decl.colorValue();
   else if (decl.d->property == "minSize") {
      int i;
      if (decl.intValue(&i)) dbg << " % " << i;
   }
   return dbg;
}

int main() {
   QCss::Parser parser(pss);
   QCss::StyleSheet styleSheet;
   if (!parser.parse(&styleSheet))
      return 1;
   for (auto rule : styleSheet.styleRules) {
      qDebug() << "** Rule **";
      for (auto sel : rule.selectors) {
        for (auto bSel : sel.basicSelectors)
           qDebug() << bSel;
      }
      for (auto decl : rule.declarations)
         qDebug() << decl;
   }
}

const char pss[] =
  "/* @include \"otherStyleSheet.pss\"; */ \
  [propertyID=\"1230000\"] {  \
    fillColor : #f3f1ed; \
    minSize : 5; \
    lineWidth : 3; \
  } \
   \
  /* sphere */ \
  [propertyID=\"124???|123000\"] {  \
    lineType : dotted; \
  } \
   \
  /* square */ \
  [propertyID=\"125???\"] { \
    lineType : thinline; \
  } \
   \
  /* ring */ \
  [propertyID=\"133???\"] { \
    lineType : thickline;  \
    /*[hasInnerRing=true] { \
      innerLineType : thinline; \
    }*/   \
  }";
