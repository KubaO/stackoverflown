#include <QtWidgets>
#include <list>

/// Removes \a n elements from the container, preferably if \a pred is true.
template <class Container, class Pred>
void remove_n_preferably_if(Container & data, int n, Pred pred) {
    int left = data.size();
    for (auto i = data.begin(), e = data.end(); n > 0 && i != e;) {
        if (left == n || pred(*i)) {
            auto j = std::next(i);
            -- left, -- n;
            for (; n > 0 && j != e && (left == n || pred(*j)); --left, --n, ++j);
            i = data.erase(i, j);
        } else {
            -- left, ++ i;
        }
    }
}

#if 0
/// Maps QObject properties to widgets
class PropertyWidgetMapper : public QObject {
    Q_OBJECT
    bool m_valid;
    QPointer<QObject> m_srcObj, m_dstObj;
    QMetaProperty m_src, m_dst;
    Q_SLOT void srcNotify() {
        QVariant value = m_src.read(m_srcObj);
        m_dst.write(m_dstObj, value);
    }
    Q_SLOT void dstNotify() {
    }

public:
    PropertyWidgetMapper(QWidget * widget, QObject * source, const QByteArray & sourceProperty, QObject * parent = 0) :
        QObject(parent), m_srcObj(source), m_dstObj(widget)
    {
        m_dst = widget->metaObject()->userProperty();
        if (! m_dst.isValid()) return;
        m_src = source->metaObject()->property(source->metaObject()->indexOfProperty(sourceProperty));
        if (! m_src.isValid() || ! m_src.hasNotifySignal()) return;
        connect(source, m_src.notifySignal(), this, staticMetaObject.method(staticMetaObject.indexOfSlot("srcNotify()")));
        if (m_dst.hasNotifySignal())
            connect(m_dstObj, m_dst.notifySignal(), this, staticMetaObject.method(staticMetaObject.indexOfSlot("dstNotify()")));
        connect(source, &QObject::destroyed, this, &QObject::deleteLater);
        connect(widget, &QObject::destroyed, this, &QObject::deleteLater);
        m_valid = true;
    }
    bool isValid() const { return m_valid; }
};
#endif

class Slave : public QObject {
    Q_OBJECT
    QBasicTimer m_timer;
    void timerEvent(QTimerEvent * ev) {
        if (ev->timerId() == m_timer.timerId()) qApp->quit();
    }
public:
    Slave(QObject *parent = 0) : QObject(parent) {
        m_timer.start(rand() % 10, this);
    }
};

class Master : public QAbstractListModel {
    Q_OBJECT
    int m_activeProcessCount = 0;
    std::list<QProcess> m_processes;
    bool m_autoStart = false;
    void notifyRowChange(int row) { emit dataChanged(index(row), index(row)); }
    Q_SLOT void started() {
        ++ m_activeProcessCount;
        notifyRowChange(RowActiveProcessCount);
        Q_ASSERT(m_activeProcessCount <= (int)m_processes.size());
    }
    Q_SLOT void finished() {
        -- m_activeProcessCount;
        notifyRowChange(RowActiveProcessCount);
        Q_ASSERT(m_activeProcessCount >= 0);
    }
    void start(QProcess & process) {
        if (process.state() == QProcess::NotRunning) process.start(qApp->applicationFilePath(), {"--slave"});
    }
    void startAll() {
        for (auto & process : m_processes) start(process);
    }
    void addProcess() {
        m_processes.emplace_back();
        QProcess & process = m_processes.back();
        connect(&process, &QProcess::started, this, &Master::started);
        connect(&process, static_cast<void (QProcess::*)(int,QProcess::ExitStatus)>(&QProcess::finished), this, &Master::finished);
        if (m_autoStart) start(process);
    }
public:
    enum { RowActiveProcessCount, RowMaxProcessCount, RowAutoStart, RowEnd };
    Master(QObject *parent = nullptr) : QAbstractListModel{parent} {
        setMaxProcessCount(QThread::idealThreadCount());
    }
    int rowCount(const QModelIndex &) const Q_DECL_OVERRIDE { return RowEnd; }
    QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE {
        if (role != Qt::DisplayRole || role != Qt::EditRole) return false;
        switch (index.row()) {
        case RowActiveProcessCount: return m_activeProcessCount;
        case RowMaxProcessCount: return int(m_processes.size());
        case RowAutoStart: return m_autoStart;
        default: return QVariant();
        }
    }
    bool setData(const QModelIndex &index, const QVariant &value, int role) Q_DECL_OVERRIDE {
        qDebug() << __FUNCTION__ << index << value << role;
        if (role != Qt::DisplayRole || role != Qt::EditRole) return false;
        switch (index.row()) {
        case RowMaxProcessCount: setMaxProcessCount(value.toInt()); return true;
        case RowAutoStart: setAutoStart(value.toBool()); return true;
        default: return false;
        }
    }
    int activeProcessCount() const { return m_activeProcessCount; }
    bool autoStart() const { return m_autoStart; }
    void setAutoStart(bool const a) {
        if (a == m_autoStart) return;
        m_autoStart = a;
        if (a) startAll();
        enum { RowActiveProcessCount, RowMaxProcessCount, RowAutoStart, RowEnd };
    }
    int maxProcessCount() const { return m_processes.size(); }
    void setMaxProcessCount(int const n) {
        if (n == maxProcessCount()) return;
        remove_n_preferably_if(m_processes, m_processes.size() - n, [](QProcess & process) { return process.state() == QProcess::NotRunning; });
        while (maxProcessCount() < n) addProcess();
        notifyRowChange(RowMaxProcessCount);
    }
};

class MasterView : public QWidget {
    QFormLayout m_layout{this};
    QLabel m_active;
    QSpinBox m_max;
    QCheckBox m_run;
    QDataWidgetMapper m_mapper;
public:
    MasterView(QWidget * parent = nullptr) : QWidget{parent} {
        m_layout.addRow("Active Process Count", &m_active);
        m_layout.addRow("Maximum # of Processes", &m_max);
        m_layout.addRow("Run Processes", &m_run);
        m_max.setMinimum(0);
        m_max.setMaximum(48);
    }
    void setSource(Master * source) {
        m_mapper.setModel(source);
        m_mapper.setOrientation(Qt::Vertical);
        m_mapper.setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
        m_mapper.addMapping(&m_active, Master::RowActiveProcessCount);
        m_mapper.addMapping(&m_max, Master::RowMaxProcessCount);
        m_mapper.addMapping(&m_run, Master::RowAutoStart);
        m_mapper.setCurrentIndex(0);
    }
};

bool argvContains(int argc, char ** argv, const char * needle)
{
    for (int i = 1; i < argc; ++ i) if (qstrcmp(argv[i], needle) == 0) return true;
    return false;
}

int main(int argc, char *argv[])
{
    if (argvContains(argc, argv, "--slave")) {
        QCoreApplication app{argc, argv};
        qsrand(QDateTime::currentMSecsSinceEpoch());
        Slave slave;
        return app.exec();
    }
    QApplication app{argc, argv};
    Master master;
    MasterView view;
    view.setSource(&master);
    view.show();
    return app.exec();
}
#include "main.moc"
