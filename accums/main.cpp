#include <Eigen/Dense>
#include <iostream>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/weighted_median.hpp>
using boost::bind;
using boost::ref;
using namespace boost::accumulators;
using namespace Eigen;

void f(int i) { std::cout << i << std::endl; }

int main(){
    accumulator_set<float, stats<tag::median > > acc1;
    accumulator_set<float, stats<tag::median >,int> acc2;

    int i = 0;
    VectorXi rw=VectorXi::Random(100);
    VectorXf rn=VectorXf::Random(100);

    std::for_each(rn.data(), rn.data()+rn.rows(), bind<void>( ref(acc1), _1 ) );
    std::for_each(i, 5, f);
#if 0
    rw=rw.cwiseAbs();
    rw(0)=1000;
    for(int i=0;i<100;i++){
        acc1(rn(i));
        acc2(rn(i),weight=rw(i));
    }
#endif

  std::cout << "         Median: " << median(acc1) << std::endl;
  std::cout << "Weighted Median: " << median(acc2) << std::endl;

  return 0;
}

