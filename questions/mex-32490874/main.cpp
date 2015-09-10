// https://github.com/KubaO/stackoverflown/tree/master/questions/mex-32490874

#include <cstddef>
#include <cmath>
#include <vector>
#include <array>

enum mxComplexity {mxREAL=0, mxCOMPLEX};
typedef bool mxLogical;
typedef int mwSize;
struct mxArray;

void mexErrMsgTxt(const char *errormsg);
bool mxIsDouble(const mxArray *pm);
double *mxGetPr(const mxArray *pm);
size_t mxGetNumberOfElements(const mxArray *pm);
const mwSize *mxGetDimensions(const mxArray *pm);
mwSize mxGetNumberOfDimensions(const mxArray *pm);
mxArray *mxCreateDoubleMatrix(mwSize m, mwSize n, mxComplexity ComplexFlag);
mxArray *mxCreateLogicalMatrix(mwSize m, mwSize n);
mxLogical *mxGetLogicals(const mxArray *array_ptr);
int mexPrintf(const char *message, ...);

int main() {}

struct Params {
    double t_crd_dbl, t_frac;
};

Params Simple_Separate_Parametrics(double t_full) {
    Params p;
    p.t_crd_dbl = floor(t_full);
    p.t_frac = t_full - p.t_crd_dbl;
    return p;
}

void Calc_Point_and_XJacobian_On_SplurfaceMap(double* P, double* J, mxLogical* Valid, unsigned int* RelCrds, double* s, double* t, double* M, double* X, unsigned int* Crds, unsigned int* GridSize) {

    mexPrintf("Inner Function Part 1 %u, %u\n", GridSize[0], GridSize[1]);

    Params p = Simple_Separate_Parametrics(*t);

    mexPrintf("Inner Function Part 2 %u, %u\n", GridSize[0], GridSize[1]);
}


void Points_and_XJacobian_on_Splurface(double* Points, double* Jacobian, mxLogical* Valid, double* s, double* t, double* M, double* X, unsigned int * GridSize, size_t NumPts) {

    double J[16];

    size_t Vpts = GridSize[1] + 3;
    size_t Xlen = (GridSize[0] + 3)*Vpts;

    std::vector<unsigned int> Crds(Xlen);
    unsigned int RelCrds[16];

    mexPrintf("Outer Function Part 1 %u, %u\n", GridSize[0], GridSize[1]);

    for (unsigned int n = 0; n < 1; ++n) {//NumPts
        Calc_Point_and_XJacobian_On_SplurfaceMap(Points + n, J, Valid + n, RelCrds, s + n, t + n, M, X, Crds.data(), GridSize);
        mexPrintf("Outer Function Part 3 %u, %u\n", GridSize[0], GridSize[1]);
    }

    mexPrintf("Outer Function Part 3 %u, %u\n", GridSize[0], GridSize[1]);
}

template <typename T> T rtnErrMsgTxt(const char * txt) { mexErrMsgTxt(txt); }

template <typename T, typename D> MxArray {
    typename D::
    double * m_data;
    size_t m_size;
    mwSize m_nDims;
    const mwSize * m_dims;
    void init(const mxArray * arr) {
        m_data = mxGetPr(arr);
        m_size = mxGetNumberOfElements(arr);
        m_nDims = mxGetNumberOfDimensions(arr);
        m_dims = mxGetDimensions(arr);
    }
public:
    typedef double * iterator;
    typedef const double * const_iterator;
    DoubleArray(const mxArray * arr) :

    {
        if (!mxIsDouble(arr)) mexErrMsgTxt("Wrong input parameter type");
        init(arr);
    }
    DoubleArray(int n, int m, mxArray * & out) {
        out = mxCreateDoubleMatrix(n, m, mxREAL);
        init(out);
    }
    size_t size() const { return m_size; }
    int dim(int i) const { return m_dims[i]; }
    double operator[](int i) const {
        if (i < m_size) return m_data[i]; else mexErrMsgTxt("Range error");
    }
    double & operator[](int i) {
        if (i < m_size) return m_data[i]; else mexErrMsgTxt("Range error");
    }
    DoubleArray offset(int i) const {
        DoubleArray arr;
        arr.m_data = m_data + i;
        arr.m_size = m_size - i;
        arr.m_nDims = m_nDims;
        arr.m_dims = m_dims;
        return arr;
    }
    iterator begin() { return m_data; }
    iterator end() { return m_data + m_size; }
    const_iterator begin() const { return m_data; }
    const_iterator end() const { return m_data + m_size; }
};

class DoubleArray {
    double * m_data;
    size_t m_size;
    mwSize m_nDims;
    const mwSize * m_dims;
    DoubleArray() = default;
    void init(const mxArray * arr) {
        m_data = mxGetPr(arr);
        m_size = mxGetNumberOfElements(arr);
        m_nDims = mxGetNumberOfDimensions(arr);
        m_dims = mxGetDimensions(arr);
    }
public:
    typedef double * iterator;
    typedef const double * const_iterator;
    DoubleArray(const mxArray * arr) {
        if (!mxIsDouble(arr)) mexErrMsgTxt("Wrong input parameter type");
        init(arr);
    }
    DoubleArray(int n, int m, mxArray * & out) {
        out = mxCreateDoubleMatrix(n, m, mxREAL);
        init(out);
    }
    size_t size() const { return m_size; }
    int dim(int i) const { return m_dims[i]; }
    double operator[](int i) const {
        if (i < m_size) return m_data[i]; else mexErrMsgTxt("Range error");
    }
    double & operator[](int i) {
        if (i < m_size) return m_data[i]; else mexErrMsgTxt("Range error");
    }
    DoubleArray offset(int i) const {
        DoubleArray arr;
        arr.m_data = m_data + i;
        arr.m_size = m_size - i;
        arr.m_nDims = m_nDims;
        arr.m_dims = m_dims;
        return arr;
    }
    iterator begin() { return m_data; }
    iterator end() { return m_data + m_size; }
    const_iterator begin() const { return m_data; }
    const_iterator end() const { return m_data + m_size; }
};

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    // [Points, Jacobian, Valid] = Function( s, t, X, GridSize)

    if (nrhs != 4)
        mexErrMsgTxt("Need four input parameters");
    if (nlhs != 3)
        mexErrMsgTxt("Need three output parameters");
    if (mxGetNumberOfElements(prhs[3]) != 2)
        mexErrMsgTxt("GridSize needs two elements");

    DoubleArray s(prhs[0]);
    DoubleArray t(prhs[1]);
    DoubleArray X(prhs[2]);
    DoubleArray inGridSize(prhs[3]);
    auto NumPts = s.size();
    std::array<size_t, 2> GridSize;
    std::copy(inGridSize.begin(), inGridSize.end(), GridSize.begin());

    mexPrintf("Start Mex %u, %u\n", GridSize[0], GridSize[1]);

    DoubleArray Points(1, NumPts, plhs[0]);
    DoubleArray Jacobian(NumPts, X.size(), plhs[1]);
    LogicalArray Valid(NumPts, 1);

    plhs[0] = mxCreateDoubleMatrix(1, NumPts, mxREAL);
    double * Points = mxGetPr(plhs[0]);
    plhs[1] = mxCreateDoubleMatrix(NumPts, X.size(), mxREAL);
    double * Jacobian = mxGetPr(plhs[1]);
    plhs[2] = mxCreateLogicalMatrix(NumPts, 1);
    mxLogical * Valid = mxGetLogicals(plhs[2]);

    std::array<double, 16> M;
    Points_and_XJacobian_on_Splurface(Points, Jacobian, Valid, s, t, M, X, GridSize, NumPts);

    mexPrintf("End Mex %u, %u\n", GridSize[0], GridSize[1]);
}


