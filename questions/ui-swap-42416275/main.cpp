// https://github.com/KubaO/stackoverflown/tree/master/questions/ui-swap-42416275
#include <QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets>
#endif

// mock uic output
namespace Ui {
struct MainMenu { void setupUi(QMainWindow*) {} };
struct PlayerMenu { void setupUi(QMainWindow*) {} };
struct OptionMenu { void setupUi(QMainWindow*) {} };
}

class MainMenuHack : public QMainWindow {
   Q_OBJECT
   enum class UiKind { MainMenu, PlayerMenu, OptionMenu };
   Ui::MainMenu uiMainMenu;
   Ui::PlayerMenu uiPlayerMenu;
   Ui::OptionMenu uiOptionMenu;
   void clearLayout(QLayout * layout) {
      if (!layout) return;
      while (layout->count()) {
         QScopedPointer<QLayoutItem> item{layout->takeAt(0)};
         if (!item)
            continue;
         delete item->widget();
         clearLayout(item->layout());
      }
   }
public:
   MainMenuHack(QWidget * parent = {}, Qt::WindowFlags flags = {}) :
      QMainWindow{parent, flags}
   {
      setAppearance(UiKind::MainMenu);
   }
   void setAppearance(UiKind kind) {
      clearLayout(layout());
      switch (kind) {
      case UiKind::MainMenu: return uiMainMenu.setupUi(this);
      case UiKind::PlayerMenu: return uiPlayerMenu.setupUi(this);
      case UiKind::OptionMenu: return uiOptionMenu.setupUi(this);
      }
   }
};

template <typename Ui>
struct ui_traits : ui_traits<decltype(&Ui::setupUi)> {};
template <typename Ui, typename Widget>
struct ui_traits<void(Ui::*)(Widget*)> {
   using widget_type = Widget;
};
template <typename Ui, typename Widget = typename ui_traits<Ui>::widget_type>
struct UiWidget : Widget, Ui {
   UiWidget(QWidget * parent = {}) : Widget{parent} { this->setupUi(this); }
   UiWidget(QStackedWidget * parent) : UiWidget{static_cast<QWidget*>(parent)} {
      parent->addWidget(this);
   }
   void setCurrent() {
      auto stack = qobject_cast<QStackedWidget*>(this->parent());
      if (stack) stack->setCurrentWidget(this);
   }
};

class MainMenu : public QStackedWidget {
   Q_OBJECT
   enum class UiKind { MainMenu, PlayerMenu, OptionMenu };
   UiWidget<Ui::MainMenu> uiMainMenu{this};
   UiWidget<Ui::PlayerMenu> uiPlayerMenu{this};
   UiWidget<Ui::OptionMenu> uiOptionMenu{this};
public:
   MainMenu(QWidget * parent = {}, Qt::WindowFlags flags = {}) :
      QStackedWidget{parent}
   {
      setWindowFlags(flags);
      setAppearance(UiKind::MainMenu);
   }
   void setAppearance(UiKind kind) {
      switch (kind) {
      case UiKind::MainMenu: return uiMainMenu.setCurrent();
      case UiKind::PlayerMenu: return uiPlayerMenu.setCurrent();
      case UiKind::OptionMenu: return uiOptionMenu.setCurrent();
      }
   }
};

class MainMenu2 : public QStackedWidget {
   Q_OBJECT
   UiWidget<Ui::MainMenu> uiMainMenu{this};
   UiWidget<Ui::PlayerMenu> uiPlayerMenu{this};
   UiWidget<Ui::OptionMenu> uiOptionMenu{this};
public:
   MainMenu2(QWidget * parent = {}, Qt::WindowFlags flags = {}) :
      QStackedWidget{parent}
   {
      setWindowFlags(flags);
      uiMainMenu.setCurrent();
   }
};

int main() {}

#include "main.moc"
