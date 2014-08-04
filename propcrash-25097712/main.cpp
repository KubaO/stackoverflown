#include <QString>

class C {
    QString m_prop1;
public:
    C() { setProp1("reset"); setProp1("reset"); }
    QString setProp1(const QString& prop1) { m_prop1 = prop1; }
} c;

int main(int, char **) {
    return 0;
}
