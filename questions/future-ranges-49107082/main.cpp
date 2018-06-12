// https://github.com/KubaO/stackoverflown/tree/master/questions/future-ranges-49107082
/* QtConcurrent will include QtCore as well */
#include <QtConcurrent>
#include <algorithm>
#include <iterator>

using result_type = double;

static result_type map_function(int instance){
   return instance * result_type(10);
}

static void sum_modifier(result_type &result, result_type value) {
   result += value;
}

static result_type sum_function(result_type result, result_type value) {
   return result + value;
}

result_type sum_approach1(int const N) {
   QVector<QFuture<result_type>> futures(N);
   int id = 0;
   for (auto &future : futures)
      future = QtConcurrent::run(map_function, id++);
   return std::accumulate(futures.cbegin(), futures.cend(), result_type{}, sum_function);
}

#include <iterator>

template <typename tag> class num_iterator : public std::iterator<tag, int, int, const int*, int> {
   int num = 0;
   using self = num_iterator;
   using base = std::iterator<tag, int, int, const int*, int>;
public:
   explicit num_iterator(int num = 0) : num(num) {}
   self &operator++() { num ++; return *this; }
   self &operator--() { num --; return *this; }
   self &operator+=(typename base::difference_type d) { num += d; return *this; }
   friend typename base::difference_type operator-(self lhs, self rhs) { return lhs.num - rhs.num; }
   bool operator==(self o) const { return num == o.num; }
   bool operator!=(self o) const { return !(*this == o); }
   typename base::reference operator*() const { return num; }
};

using num_f_iterator = num_iterator<std::forward_iterator_tag>;

result_type sum_approach2(int const N) {
   auto results = QtConcurrent::blockingMapped<QVector<result_type>>(num_f_iterator{0}, num_f_iterator{N}, map_function);
   return std::accumulate(results.cbegin(), results.cend(), result_type{}, sum_function);
}

using num_b_iterator = num_iterator<std::bidirectional_iterator_tag>;

result_type sum_approach3(int const N) {
   auto results = QtConcurrent::blockingMapped<QVector<result_type>>(num_b_iterator{0}, num_b_iterator{N}, map_function);
   return std::accumulate(results.cbegin(), results.cend(), result_type{}, sum_function);
}

result_type sum_approach4(int const N) {
   return QtConcurrent::blockingMappedReduced(num_b_iterator{0}, num_b_iterator{N},
                                              map_function, sum_modifier);
}

using num_r_iterator = num_iterator<std::random_access_iterator_tag>;

result_type sum_approach5(int const N) {
   return QtConcurrent::blockingMappedReduced(num_r_iterator{0}, num_r_iterator{N},
                                              map_function, sum_modifier);
}

#include <numeric>

result_type sum_approach6(int const N) {
   QVector<int> sequence(N);
   std::iota(sequence.begin(), sequence.end(), 0);
   return QtConcurrent::blockingMappedReduced(sequence, map_function, sum_modifier);
}

template <typename F> void benchmark(F fun, double const N) {
   QElapsedTimer timer;
   timer.start();
   auto result = fun(N);
   qDebug() << "sum:" << fixed << result << "took" << timer.elapsed()/N << "ms/item";
}

int main() {
   const int N = 1000000;
   benchmark(sum_approach1, N);
   benchmark(sum_approach2, N);
   benchmark(sum_approach3, N);
   benchmark(sum_approach4, N);
   benchmark(sum_approach5, N);
   benchmark(sum_approach6, N);
}
