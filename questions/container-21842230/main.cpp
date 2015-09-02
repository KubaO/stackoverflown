#include <iostream>
#include <cassert>
#include <memory>
using namespace std;

template <class T> class container {
   container(const container &) = delete;
public:
   container(); // constructor
   // Post-condition: count is set to -1 and dynamic array size set to 5
   void insert(T &n);
   //  Pre-condition: a value is passed to the function
   // Post-condition: if data array is not full, increment count by 1 and insert the value in the array
   // Otherwise, display "Container full! No insertion is made."
   void remove();
   //Post-condition: if data array is not empty, remove the data[count] element in data array and decrement count by 1;
   //otherwise, display a message "Container empty! Nothing is removed from it."
   void display();
   // Post-condition: if the array is not empty, displays all values in data array similar to the sample output;
   //Otherwise, display the message “Container is now empty!"
   template <typename Q> static void fillarray(container<Q> & c);
   //pre-condition: a container c is passed to the function
   //post-condition: dynamic array of chosen type is created and filled continuously with user entered values
private:
   bool empty;
   //Post-condition: returns true is the array is empty, otherwise returns false
   int count;   // indicates how many values have been inserted
   int max;
   std::unique_ptr<T[]> data; // dynamically allocated array used to store or contain the inserted values
};
template <class T> container<T>::container() :
   empty(true),
   count(-1),
   max(5),
   data(new T[max])
{}
//template <class T> container<T>::~container() {}
template <class T> void container<T>::insert(T &n)
{
   if (count >= (max - 1))
   {
      max = max * 2;
      cout << "\nContainer full! Array size is increased by " << max/2  << ".";
      T *temp = new T[max];
      for (int i = 0; i < count; i++)
         temp[i] = data[i];
      delete [] data;
      data = temp;
      count++;
      data[count] = n;
   }
   else
      count++;
   data[count] = n;
}
template <class T> void container<T>::remove()
{
   empty = count < 0;
   if (empty == 1)
   {
      cout << "\nContainer empty! Nothing is removed from it.";}
   else
   {
      count--;
      T *temp1 = new T[max];
      assert(temp1 != NULL);
      for (int i = 0; i < count; i++)
         temp1[i] = data[i];
      delete [] data;
      data = temp1;
   }
}
template <class T> void container<T>::display()
{
   empty = count < 0;
   if (empty == 1)
   {
      cout << "\nContainer is now empty!";}
   else
   {
      for (int i = 0; i <= count; ++i)
         cout << " " << data[i];
   }
}
template <class T> template <typename Q> void container<T>::fillarray(container<Q> & c)
{
   char ans;
   do
   {
      T value;
      cout << "\nEnter a value:";
      cin >> value;
      c.insert(value);
      cout << "\nAfter inserting, the \"container\" contains:" << endl;
      c.display();
      cout << "\nEnter more? (Y/y or N/n)";
      cin >> ans;
   } while (ans == 'Y' || ans == 'y');
   for (int i = 0; i <= count; i++)
   {
      c.remove();
      cout << "\nAfter removing a value from it, the \"container\" contains:" << endl;
      c.display();
      cout << endl;
   }
}

// The main driver function to be used to test implementation
int main()
{
   char choice;
   cout << "\nEnter S for string container, D for double";
   cin >> choice;
   if (choice == 'S' || choice == 's')
   {
      container<string> c;
      c.display();
      c.fillarray(c);
   }
   else if(choice == 'D' || choice == 'd')
   {
      container<double> c;
      c.display();
      c.fillarray(c);
   }
   return 0;
}
