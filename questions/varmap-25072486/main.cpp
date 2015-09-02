#include <iostream>
#include <string>
#include <map>

using namespace std;


template <typename First, typename ... Rest> class storage {
    void * m_data;
    bool m_own;
    int m_which;
    template <typename T, typename ...P> struct which_impl;
    template <typename T, typename F, typename ...R> struct which_impl<T, F, R...> {
        static int which(int n) { return which_impl<T, R...>::which(n+1); }
    };
    template <typename T, typename ...R> struct which_impl<T, T, R...> {
        static int which(int n) { return n; }
    };
    template <typename ...P> struct impl;
    template <typename F> struct impl<F> {
        static void destruct(int, void * data) { delete reinterpret_cast<F*>(data); }
        static void * copy(int, void * src) { return new F(*reinterpret_cast<F*>(src)); }
        static const type_info & type(int) { return typeid(F); }
    };
    template <typename F, typename ... R> struct impl<F,R...> {
        static void destruct(int n, void * data) {
            if (n==0) delete reinterpret_cast<F*>(data);
            impl<R...>::destruct(n-1, data);
        }
        static void * copy(int n, void * src) {
            if (n==0) return new F(*reinterpret_cast<F*>(src));
            return impl<R...>::copy(n-1, src);
        }
        static const type_info & type(int n) {
            if (n==0) return typeid(F);
            return impl<R...>::type(n-1);
        }
    };
    void destruct() { if (m_own) impl<First, Rest...>::destruct(m_which, m_data); }
public:
    storage() : m_data(new First), m_own(true), m_which(0) {}
    storage(const storage & other) : m_data(other.m_own ? impl<First, Rest...>::copy(other.m_which, other.m_data) : other.m_data),
        m_own(other.m_own), m_which(other.m_which) {}
    storage(storage && other) : m_data(other.m_data), m_own(other.m_data), m_which(other.m_which) {
        other.m_data = 0;
        other.m_own = true;
        other.m_which = 0;
    }
    template <typename T> storage(T & other) : m_data(&other), m_own(false), m_which(which_impl<T,First,Rest...>::which(0)) {}
    template <typename T> storage(const T & other) : m_data(new T(other)), m_own(true), m_which(-1) {}
    template <typename T> storage(T && other) : m_data(new T(std::move(other))), m_own(true), m_which(-1) {}
    template <typename T> storage & operator=(T & other) {
        destruct();
        m_data = &other;
        m_own = false;
        m_which = -1;
        return *this;
    }
    template <typename T> storage & operator=(const T & other) {
        destruct();
        m_data = new T(other);
        m_own = true;
        m_which = -1;
        return *this;
    }
    template <typename T> storage & operator=(T && other) {
        destruct();
        m_data = new T(std::move(other));
        m_own = true;
        m_which = -1;
        return * this;
    }

    ~storage() { destruct(); }
    int which() const { return m_which; }
    const std::type_info & type() const { return impl<First, Rest...>::type(m_which); }
};

template <typename First, typename ...Rest> template<> storage<First,Rest...>::storage<storage<First,Rest...>>(const storage<First,Rest> & other) {}

int main()
{
    storage<std::string, int> s1;
    storage<std::string, int> s2(s1);
    cout << "Hello World!" << endl;
    return 0;
}

