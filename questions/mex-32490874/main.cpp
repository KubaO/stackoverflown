// https://github.com/KubaO/stackoverflown/tree/master/questions/mex-32490874

#include <cstddef>
#include <cmath>
#include <vector>
#include <array>

//

enum mxComplexity {mxREAL=0, mxCOMPLEX};
typedef bool mxLogical;
typedef int mwSize;
struct mxArray;

void mexErrMsgTxt(const char *errormsg);
void mexErrMsgIdAndTxt(const char *errorid, const char *errormsg, ...);
bool mxIsDouble(const mxArray *pm);
bool mxIsLogical(const mxArray *pm);
double *mxGetPr(const mxArray *pm);
size_t mxGetNumberOfElements(const mxArray *pm);
const mwSize *mxGetDimensions(const mxArray *pm);
mwSize mxGetNumberOfDimensions(const mxArray *pm);
mxArray *mxCreateDoubleMatrix(mwSize m, mwSize n, mxComplexity ComplexFlag);
mxArray *mxCreateLogicalMatrix(mwSize m, mwSize n);
mxLogical *mxGetLogicals(const mxArray *array_ptr);
int mexPrintf(const char *message, ...);

int main() {}

//

template <typename T> struct Traits;

template <typename T, class Tr = Traits<T>> class MxArray {
   const mxArray * m_data;
   T * m_ptr;
public:
   typedef T * iterator;
   typedef const T * const_iterator;
   MxArray(const mxArray * data[], int index) :
      m_data(Tr::check(data, index)),
      m_ptr(Tr::ptr(m_data))
   {}
   MxArray(int n, int m, mxArray * &out) :
      m_data(out = Tr::create(n, m)),
      m_ptr(Tr::ptr(out))
   {}
   size_t size() const { return mxGetNumberOfElements(m_data) - offset(); }
   int n_dims() const { return mxGetNumberOfDimensions(m_data); }
   int dim(int i) const { return mxGetDimensions(m_data)[i]; }
   inline ptrdiff_t offset() const { return m_ptr - Tr::ptr(m_data); }
   T operator[](ptrdiff_t i) const { return m_ptr[i]; }
   T & operator[](ptrdiff_t i) { return m_ptr[i]; }
   MxArray & operator+=(ptrdiff_t offset) {
      m_ptr += offset;
      return *this;
   }
   friend T operator+(MxArray lhs, const MxArray & rhs) {
      return lhs += rhs;
   }
   void resetOffset() { m_ptr = Tr::ptr(m_data); }
   iterator begin() { return m_ptr; }
   iterator end() { return m_ptr + size(); }
   const_iterator begin() const { return m_ptr; }
   const_iterator end() const { return m_ptr + size(); }
};

const char kMsgId[] = "SO:MexExample";

template <> struct Traits<double> {
   static const mxArray * check(const mxArray * data[], int index) {
      if (! mxIsDouble(data[index]))
         mexErrMsgIdAndTxt(kMsgId, "Expected real type for input parameter #%d", index);
      return data[index];
   }
   static double * ptr(const mxArray * data) {
      return mxGetPr(data);
   }
   static mxArray * create(int n, int m) {
      return mxCreateDoubleMatrix(n, m, mxREAL);
   }
};

template <> struct Traits<mxLogical> {
   static const mxArray * check(const mxArray * data[], int index) {
      if (! mxIsLogical(data[index]))
         mexErrMsgIdAndTxt(kMsgId, "Expected logical type for input parameter #%d", index);
      return data[index];
   }
   static mxLogical * ptr(const mxArray * data) {
      return mxGetLogicals(data);
   }
   static mxArray * create(int n, int m) {
      return mxCreateLogicalMatrix(n, m);
   }
};

typedef MxArray<double> DoubleArray;
typedef MxArray<mxLogical> LogicalArray;

struct Params {
   DoubleArray s, t, X, inGridSize;
   size_t NumPts;
   std::array<size_t, 2> GridSize;
   DoubleArray Points, Jacobian;
   LogicalArray Valid;
   std::array<double, 16> M;

   Params(mxArray * plhs[], const mxArray * prhs[]) :
      s(prhs, 0),
      t(prhs, 1),
      X(prhs, 2),
      inGridSize(prhs, 3),
      NumPts(s.size()),
      Points(1, NumPts, plhs[0]),
      Jacobian(NumPts, X.size(), plhs[1]),
      Valid(NumPts, 1, plhs[2])
   {
      std::copy(inGridSize.begin(), inGridSize.end(), GridSize.begin());
   }
};

struct PointParams {
   unsigned int Vpts;
   unsigned int Xlen;
   std::array<double, 16> J;
   std::vector<unsigned int> Crds { Xlen };
   std::array<unsigned int, 16> RelCrds;

   PointParams(const Params & p) :
      Vpts(p.GridSize[1] + 3),
      Xlen((p.GridSize[0] + 3)*Vpts)
   {}
};

struct Parametrics {
   double t_crd_dbl, t_frac;
};

Parametrics Simple_Separate_Parametrics(double t_full) {
   Parametrics p;
   p.t_crd_dbl = floor(t_full);
   p.t_frac = t_full - p.t_crd_dbl;
   return p;
}

void Calc_Point_and_XJacobian_On_SplurfaceMap(Params & p, PointParams & pp) {
   mexPrintf("Begin Inner Function %u, %u\n", p.GridSize[0], p.GridSize[1]);

   auto pms = Simple_Separate_Parametrics(p.t[0]);

   mexPrintf("End Inner Function %u, %u\n", p.GridSize[0], p.GridSize[1]);
}

void Points_and_XJacobian_on_Splurface(Params p) {
   mexPrintf("Begin Outer Function %u, %u\n", p.GridSize[0], p.GridSize[1]);

   for (unsigned int n = 0; n < 1; ++n) {//NumPts
      PointParams pp(p);
      Calc_Point_and_XJacobian_On_SplurfaceMap(p, pp);
      p.Points += 1;
      p.Valid += 1;
      p.s += 1;
      p.t += 1;
   }

   mexPrintf("End Outer Function %u, %u\n", p.GridSize[0], p.GridSize[1]);
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
   // [Points, Jacobian, Valid] = Function( s, t, X, GridSize)

   if (nrhs != 4)
      mexErrMsgTxt("Need four input parameters");
   if (nlhs != 3)
      mexErrMsgTxt("Need three output parameters");
   if (mxGetNumberOfElements(prhs[3]) != 2)
      mexErrMsgTxt("GridSize needs two elements");

   Params p { plhs, prhs };
   mexPrintf("Begin Mex %u, %u\n", p.GridSize[0], p.GridSize[1]);
   Points_and_XJacobian_on_Splurface(p);
   mexPrintf("End Mex %u, %u\n", p.GridSize[0], p.GridSize[1]);
}


