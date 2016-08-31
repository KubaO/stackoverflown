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
    for (auto & combo : combos) {
        layout.addWidget(&combo);
        combo.setModel(&model);
        QObject::connect(&combo,
                         static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                         [&combos,&combo](int index){
            for (auto & combo2 : combos)
                if (&combo2 != &combo)
                    combo2.setCurrentIndex(index);
        });
    }
    ui.show();

    return app.exec();
}
