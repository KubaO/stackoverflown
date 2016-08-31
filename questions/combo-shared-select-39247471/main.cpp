// https://github.com/KubaO/stackoverflown/tree/master/questions/combo-shared-select-39247471
#include <QtWidgets>
#include <array>

int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    QStringListModel model;
    model.setStringList({ "foo", "bar", "baz "});

    QWidget ui;
    QHBoxLayout layout{&ui};
    std::array<QComboBox, 3> combos;
    // setIndices could be a method in a class
    auto setIndices = [&combos](int index) {
        for (auto & combo : combos)
            combo.setCurrentIndex(index);
    };
    for (auto & combo : combos) {
        using namespace std::placeholders;
        layout.addWidget(&combo);
        combo.setModel(&model);
        QObject::connect(&combo,
                         static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                         setIndices);
    }
    ui.show();

    return app.exec();
}
