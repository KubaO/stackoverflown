// https://github.com/KubaO/stackoverflown/tree/master/questions/lineedit-frame-37831979
#include <QtWidgets>
#include <QtUiTools>

const char ui[] = R"EOF(
 <ui version="4.0">
  <class>Form</class>
  <widget class="QWidget" name="Form">
   <layout class="QFormLayout" name="formLayout">
    <item row="0" column="0">
     <widget class="QLabel" name="label">
      <property name="text">
       <string>TextLabel</string>
      </property>
     </widget>
    </item>
    <item row="0" column="1">
     <widget class="QLineEdit" name="lineEdit"/>
    </item>
   </layout>
  </widget>
  <resources/>
  <connections/>
 </ui>)EOF";

int main(int argc, char ** argv) {
   QApplication app{argc, argv};
   QBuffer buf;
   buf.setData(ui, sizeof(ui));
   auto w = QUiLoader().load(&buf);
   auto & layout = *static_cast<QFormLayout*>(w->layout());
   QLineEdit edit;
   layout.addRow("Edit here", &edit);
   w->show();
   return app.exec();
}
