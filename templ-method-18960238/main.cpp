#include <iostream>
#include <cassert>
using namespace std;

class A
{
    // we cdo magic without sprinking any dust, but optionally we could sprinkle some beforehand
    void virtual sprinkle() {};
    void virtual magic(int) = 0;
public:
    void doSomeMagic(int power) {
        assert(power > 3 and power < 8);
        sprinkle();
        magic(power);
    }
};

class B: public A
{
    void magic(int power) {
        cout << "B: did magic of with power=" << power << endl;
    }

};

class C : public B
{
    void sprinkle() {
        cout << "C: also sprinked some dust before doing the magic" << endl;
    }
};

int main()
{
    B b;
    C c;
    b.doSomeMagic(5);
    c.doSomeMagic(6);
    return 0;
}

