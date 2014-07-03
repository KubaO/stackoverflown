#include <iostream>

using namespace std;

int main()
{
    int value;
    do
    {
        cout << "\nPlease enter a homework score<'Ctrl-z' when done>: ";
        cin >> value;

        while(cin.fail() || value < 0)
        {
            cout << "not a valid positive numerical value. try again. \n";
            cin.clear();
            cin.ignore(numeric_limits<int>::max(),'\n');
            cout << "\nplease enter a valid homework score<'Ctrl-z' when done>: ";
            cin >> value;
        }
        //assignmentScore.push_back(value);

    } while(cin >> value);
    return 0;
}

