#include <cstring>

class Matrix {
    double m_data[4][4];
public:
    typedef double (*Data)[4];
    Matrix() {}
    Matrix(const Matrix & other) { memcpy(m_data, other.m_data, sizeof m_data); }
    Matrix & operator=(const Matrix & other)  { memcpy(m_data, other.m_data, sizeof m_data); return *this; }
    Matrix & operator=(const Data other)  { memcpy(m_data, other, sizeof m_data); return *this; }
    operator Data() { return m_data; }
};

int main()
{
    double mat1[4][4];
    Matrix mat2;
    mat2[3][3] = 1;
    mat2 = mat1;
    return 0;
}

