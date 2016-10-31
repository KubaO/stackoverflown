// https://github.com/KubaO/stackoverflown/tree/master/questions/ios-timing-40304260
#include <fstream>
#include <iostream>
#include <chrono>
using namespace std;

int main() {
   const int N = 5000000;
   const char kFile1[] = "tmp1.txt", kFile2[] = "tmp2.txt";

   auto start = chrono::system_clock::now();
   {
      ofstream fOut;
      fOut.open(kFile1);
      for (int i = 0; i < N; ++i)
         // !! DO NOT use endl here!
         fOut << i-N << ' ' << N-i << '\n';
   }
   auto t1 = chrono::system_clock::now();
   cerr << "t1=" << chrono::duration<double>(t1-start).count() << "s" << endl;

   double vert, adj;

   {
      ifstream fIn;
      ofstream fOut;
      fIn.open(kFile1);
      fOut.open(kFile2);
      while (fIn >> vert && fIn >> adj)
         // !! DO NOT use endl here!
         fOut << vert << ' ' << adj << '\n';
   }
   auto t2 = chrono::system_clock::now();
   cerr << "t2=" << chrono::duration<double>(t2-t1).count() << "s" << endl;

   {
      ifstream fIn;
      fIn.open(kFile1);
      while (fIn >> vert && fIn >> adj)
         // !! DO NOT use endl here!
         cout << vert << ' ' << adj << '\n';
   }
   auto t3 = chrono::system_clock::now();
   cerr << "t3=" << chrono::duration<double>(t3-t2).count() << "s" << endl;
}
