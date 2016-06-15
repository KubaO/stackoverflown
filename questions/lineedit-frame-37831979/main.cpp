// https://github.com/KubaO/stackoverflown/tree/master/questions/lineedit-frame-37831979
#include <QtWidgets>
#include <QtUiTools>

const char ui[] = R"EOF(
  <ui version="4.0">
   <class>DatasetWidget</class>
   <widget class="QWidget" name="DatasetWidget">
    <layout class="QVBoxLayout" name="verticalLayout">
     <item>
      <widget class="QLabel" name="lblCaptionDatensatz">
       <property name="text">
        <string>Datensatz:</string>
       </property>
      </widget>
     </item>
     <item>
      <widget class="QLabel" name="lblDatensatz">
       <property name="text">
        <string>C-XXXX-XXX</string>
       </property>
      </widget>
     </item>
     <item>
      <widget class="QLineEdit" name="leDescriptor">
       <property name="maxLength">
        <number>7</number>
       </property>
      </widget>
     </item>
     <item>
      <layout class="QFormLayout" name="formDisp"></layout>
     </item>
    </layout>
   </widget>
  <connections/>
  </ui>)EOF";

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QBuffer buf;
   buf.setData(ui, sizeof(ui));
   auto w = QUiLoader().load(&buf);
   auto & layout = *w->findChild<QFormLayout*>("formDisp");
   QLineEdit edit1, edit2;
   layout.addRow("Edit1", &edit1);
   layout.addRow("Edit2", &edit2);
   edit1.setMinimumHeight(50);
   edit2.setMinimumHeight(100);
   w->show();
   return app.exec();
}
