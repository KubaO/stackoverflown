// https://github.com/KubaO/stackoverflown/tree/master/questions/threaded-data-39252150
#include <QtWidgets>
#include <QtConcurrent>

class MyData {
public:
    MyData() {}
    /// Loads and pre-processes the data
    bool load(const QString & file) { QThread::sleep(5); return true; }
    /// Transforms the data
    void transform() { QThread::sleep(2); }
};

using MyDataPtr = std::shared_ptr<MyData>;
Q_DECLARE_METATYPE(MyDataPtr)

class GuiClass : public QWidget {
    Q_OBJECT
    MyDataPtr data;
    Q_SIGNAL void hasData(const MyDataPtr &);
public:
    GuiClass() {
        connect(this, &GuiClass::hasData,
                this, [this](const MyDataPtr & newData){ data = newData; });
    }
    Q_SLOT void makeData(const QString & file) {
        QtConcurrent::run([=]{
            auto data = std::make_shared<MyData>();
            if (data->load(file))
                emit hasData(data);
        });
    }
    Q_SLOT void transformData1() {
        MyDataPtr copy = std::make_shared<MyData>(*data);
        QtConcurrent::run([=]{
            copy->transform();
            emit hasData(copy);
        });
    }
    Q_SLOT void transformData2() {
        MyDataPtr data = this->data;
        this->data.reset();
        QtConcurrent::run([=]{
            data->transform();
            emit hasData(data);
        });
    }
};

class GuiClassVal : public QWidget {
    Q_OBJECT
    MyData data;
    struct DataEvent : public QEvent {
        static const QEvent::Type type;
        MyData data;
        DataEvent(const MyData & data) : QEvent{type}, data{data} {}
        DataEvent(MyData && data) : QEvent{type}, data{std::move(data)} {}
    };
    /// This method is thread-safe.
    void setData(MyData && data) {
        QCoreApplication::postEvent(this, new DataEvent{std::move(data)});
    }
    /// This method is thread-safe.
    void setData(const MyData & data) {
        QCoreApplication::postEvent(this, new DataEvent{data});
    }
    bool event(QEvent *event) override {
        if (event->type() == DataEvent::type) {
            data.~MyData();
            new (&data) MyData{std::move(static_cast<DataEvent*>(event)->data)};
            return true;
        }
        return QWidget::event(event);
    }
    void transformInEvent(DataEvent * ev){
        ev->data.transform();
        QCoreApplication::postEvent(this, ev);
    };
public:
    Q_SLOT void makeData(const QString & file) {
        QtConcurrent::run([=]{
            MyData data;
            if (data.load(file)) setData(std::move(data));
        });
    }
    Q_SLOT void transformData1() {
        QtConcurrent::run(this, &GuiClassVal::transformInEvent, new DataEvent{data});
    }
    Q_SLOT void transformData2() {
        QtConcurrent::run(this, &GuiClassVal::transformInEvent, new DataEvent{std::move(data)});
    }
};
const QEvent::Type GuiClassVal::DataEvent::type = (QEvent::Type)QEvent::registerEventType();


int main(int argc, char ** argv) {
    QApplication app{argc, argv};
    qRegisterMetaType<MyDataPtr>();
    return app.exec();
}
