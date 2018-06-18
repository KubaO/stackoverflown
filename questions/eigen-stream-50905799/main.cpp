// https://github.com/KubaO/stackoverflown/tree/master/questions/eigen-stream-50905799
#include <QtCore>
#include <Eigen/Eigen>
#include <type_traits>

// Implementation

namespace detail {
enum EigenType : quint8 {
   Double =     0x00,
   CDouble =    0x01,
   ScalarMask = 0x0F,
   Vector =     0x40,   // d x 1
   RowVector =  0x80,   // 1 x d
   Matrix =     0xC0,   // d x d
   StructureMask = 0xC0
};

template <typename T> void dumpElement(QDataStream &out, T el)
{ out << (double)el; }

template <typename T> void dumpElement(QDataStream &out, const std::complex<T> &el)
{ out << (double)el.real() << (double)el.imag(); }

template <typename T> void readDouble(QDataStream &in, T &el)
{ double v; in >> v; el = v; }

template <typename T> void readDouble(QDataStream &in, std::complex<T> &el)
{ double v; in >> v; el.real(v); el.imag(0); }

template <typename T> void readComplex(QDataStream &in, T &el)
{ double re, im; in >> re >> im; el = re; }

template <typename T> void readComplex(QDataStream &in, std::complex<T> &el)
{ double re, im; in >> re >> im; el.real(re); el.imag(im); }
} // namespace detail

template <typename D>
QDataStream &operator<<(QDataStream &out, const Eigen::MatrixBase<D> &m) {
   quint8 type = 0;
   if (std::is_integral<typename D::Scalar>::value ||
       std::is_floating_point<typename D::Scalar>::value)
      type = detail::Double;
   else if (std::is_same<std::complex<float>, typename D::Scalar>::value ||
            std::is_same<std::complex<double>, typename D::Scalar>::value)
      type = detail::CDouble;
   else {
      out.setStatus(QDataStream::WriteFailed);
      return out;
   }
   if (m.cols() == 1)
      out << (type |= detail::Vector) << (quint32)m.rows();
   else if (m.rows() == 1)
      out << (type |= detail::RowVector) << (quint32)m.cols();
   else
      out << (type |= detail::Matrix) << (quint32)m.rows() << (quint32)m.cols();

   for (int i = 0; i < m.rows(); i ++)
      for (int j = 0; j < m.cols(); j ++)
         detail::dumpElement(out, m(i, j));
   return out;
}

template <typename D>
QDataStream &operator>>(QDataStream &in, Eigen::MatrixBase<D> &m) {
   using std::swap;
   quint8 type;
   quint32 rows, cols;
   in >> type;
   if (in.status() != QDataStream::Ok)
      return in;
   bool const complex = (type & detail::ScalarMask) == detail::CDouble;
   quint8 t = type & detail::StructureMask;
   if (t == detail::Vector) {
      cols = 1;
      in >> rows;
   }
   else if (t == detail::RowVector) {
      rows = 1;
      in >> cols;
   }
   else if (t == detail::Matrix) {
      in >> rows >> cols;
   }
   else {
      in.setStatus(QDataStream::ReadCorruptData);
      return in;
   }
   if (in.status() != QDataStream::Ok)
      return in;
   if ((rows == 1 && m.ColsAtCompileTime == 1) || (cols == 1 && m.RowsAtCompileTime == 1))
      swap(rows, cols); // row- and column-vectors are interchangeable
   m.derived().resize(rows, cols);
   if (m.rows() == rows && m.cols() == cols) {
      if (!complex) {
         for (uint i = 0; i < rows; i ++)
            for (uint j = 0; j < cols; j ++)
               detail::readDouble(in, m(i, j));
      } else {
         for (uint i = 0; i < rows; i ++)
            for (uint j = 0; j < cols; j ++)
               detail::readComplex(in, m(i, j));
      }
   } else {
      double dummy;
      for (uint i = 0; i < (rows*cols*(complex ? 2 : 1)); ++i)
         in >> dummy;
   }
   return in;
}

// Test Harness

template <typename T> QDebug operator<<(QDebug d, const std::complex<T> &c) {
   return d << c.real() << ";" << c.imag();
}

template <typename T1, typename T2> struct Eigen::internal::cast_impl<std::complex<T1>, T2> {
   EIGEN_DEVICE_FUNC static inline T2 run(const std::complex<T1> &x) { return x.real(); }
};

template <typename T1, typename T2 = T1, typename T3 = T1> void test() {
   QByteArray data;
   QDataStream ds(&data, QIODevice::ReadWrite);

   using namespace Eigen;
   Matrix<T1,3,1> const v1 = Matrix<T3,3,1>::Random().template cast<T1>();
   Matrix<T1,1,3> const v2 = Matrix<T3,1,3>::Random().template cast<T1>();
   Matrix<T1,4,4> const m1 = Matrix<T3,4,4>::Random().template cast<T1>();
   Matrix<T1,Dynamic,Dynamic> const m2 = Matrix<T3,Dynamic,Dynamic>::Random(4, 4).template cast<T1>();
   Matrix<T1,Dynamic,Dynamic> const m3 = Matrix<T3,Dynamic,Dynamic>::Random(2, 5).template cast<T1>();
   ds << v1 << v1 << v2 << v2 << m1 << m1 << m2 << m2 << m3;

   Matrix<T2,3,1> iv3 = Matrix<T2,3,1>::Random();
   Matrix<T2,1,3> irv3;
   Matrix<T2,4,4> im4;
   Matrix<T2,Dynamic,Dynamic> imX;
   ds >> iv3;
   Q_ASSERT(iv3.template cast<T3>() != v1.template cast<T3>());
   Q_ASSERT(ds.status() != QDataStream::Ok);

   ds.device()->reset();
   ds.resetStatus();
   Q_ASSERT(ds.status() == QDataStream::Ok);
   ds >> iv3 >> irv3;
   Q_ASSERT(ds.status() == QDataStream::Ok);
   Q_ASSERT(iv3.template cast<T3>() == v1.template cast<T3>());
   Q_ASSERT((irv3.template cast<T3>() == Eigen::Matrix<T3,1,3>(v1.template cast<T3>())));
   ds >> irv3 >> iv3;
   Q_ASSERT(irv3.template cast<T3>() == v2.template cast<T3>());
   Q_ASSERT((iv3.template cast<T3>() == Eigen::Matrix<T3,3,1>(v2.template cast<T3>())));

   ds >> im4 >> imX;
   Q_ASSERT(im4.template cast<T3>() == m1.template cast<T3>());
   Q_ASSERT(imX.template cast<T3>() == m1.template cast<T3>());
   ds >> im4 >> imX;
   Q_ASSERT(im4.template cast<T3>() == m2.template cast<T3>());
   Q_ASSERT(imX.template cast<T3>() == m2.template cast<T3>());
   ds >> imX;
   Q_ASSERT(imX.template cast<T3>() == m3.template cast<T3>());
}

int main() {
   test<int>();
   test<float>();
   test<double>();
   test<std::complex<float>>();
   test<std::complex<double>>();

   test<short, float>();
   test<float, short, short>();
   test<int, double>();
   test<double, int, int>();
   test<float, double>();
   test<double, float, float>();

   test<float, std::complex<float>>();
   test<float, std::complex<double>>();
   test<double, std::complex<float>, float>();
   test<double, std::complex<double>>();
   test<std::complex<float>, float, float>();
   test<std::complex<double>, float, float>();
   test<std::complex<float>, double, double>();
   test<std::complex<double>, double, double>();

   test<short, std::complex<float>>();
   test<std::complex<float>, short, short>();
   test<int, std::complex<double>>();
   test<std::complex<double>, int, int>();
}
