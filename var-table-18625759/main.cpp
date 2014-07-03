#include <boost/variant.hpp>
#include <sstream>
#include <string>
#include <vector>

int main(int, char **)
{
    using namespace std;
    typedef vector<int> IntColumn;
    typedef vector<string> StringColumn;
    typedef boost::variant<vector<int>, vector<string> > Column;
    vector<Column> table;

    const int rows = 100;
    table.push_back(vector<int>(rows));
    table.push_back(vector<string>(rows));

    for (int row = 0; row < rows; ++ row) {
        stringstream str;
        str << "row " << row;
        table[row].get<IntColumn>[0] = row;
        table[row][1] = str.str();
    }

    return 0;
}
