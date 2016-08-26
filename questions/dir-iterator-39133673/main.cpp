// https://github.com/KubaO/stackoverflown/tree/master/questions/dir-iterator-39133673
#include <QtCore>
#include <boost/optional.hpp>
#include <boost/iterator/iterator_facade.hpp>

class QDir_iterator : public
        boost::iterator_facade<QDir_iterator, QString,
        boost::forward_traversal_tag, QString>
{
    friend class boost::iterator_core_access;
    boost::optional<QDirIterator &> it;
    bool equal(const QDir_iterator & other) const {
        return **this == *other;
    }
    QString dereference() const {
        return it != boost::none ? it->filePath() : QString{};
    }
    void increment() {
        it->next();
    }
public:
    QDir_iterator() = default;
    QDir_iterator(QDirIterator & dir) : it{dir} {
        it->next();
    }
};

QDir_iterator begin(QDirIterator & dir) { return QDir_iterator{dir}; }
QDir_iterator end(QDirIterator &) { return QDir_iterator{}; }

void test1() {
    auto curr = QDir::current();
    auto entries = curr.entryList();
    int i = 0;
    for (auto dir : QDirIterator{curr}) ++i;
    Q_ASSERT(i == entries.count());
}

template <typename F>
void iterate(QDirIterator && it, F && fun) {
     while (it.hasNext())
        fun(it.next());
}

void test2() {
    auto curr = QDir::current();
    auto entries = curr.entryList();
    int i = 0, j = 0;
    iterate({curr}, [&](const QString &){ ++i; });
    Q_ASSERT(entries.length() == i);
    iterate({curr}, [&](const QString &){ ++j; });
    Q_ASSERT(entries.length() == j);
}

int main() {
    test1();
    test2();
}
