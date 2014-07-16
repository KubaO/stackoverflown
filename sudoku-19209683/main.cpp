#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cassert>

using std::endl;
using std::cout;

class Board {
    int m_board[4][4];
public:
    Board() { clear(); }
    int at(int x, int y) const {
        assert(x < 4 && y < 4);
        return m_board[x][y];
    }
    int & at(int x, int y) {
        assert(x < 4 && y < 4);
        return m_board[x][y];
    }
    void clear() { memset(m_board, 0, sizeof(m_board)); }
    void set(int x, int y, int val) {
        assert(x < 4 && y < 4);
        m_board[x][y] = val;
    }
    bool isEmpty(int x, int y) const {
        assert(x < 4 && y < 4);
        return m_board[x][y] == 0;
    }
};

std::ostream& operator<< (std::ostream& stream, const Board& matrix) {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            stream << matrix.at(x,y) << " ";
        }
        stream << endl;
    }
    return stream;
}

int main()
{
    // Declaring different Variables
    Board board;
    int Sudoku[10][10]= {{0}};
    int SPositionX,SPositionY,Number;
    int rPositionX,rPositionY;
    int Counter,MemoA,MemoB;
    int CounterA,CounterB;
    bool PositionGood;
    srand(time(0));
    std::ofstream Out("TexT.txt");
    /***/
    for(SPositionX=0; SPositionX<=2; SPositionX++) // Box Counter X
        for(SPositionY=0; SPositionY<=2; SPositionY++) // Box Counter Y
        {
            cout << board;
            board.clear();
            cout << SPositionX << "-" << SPositionY << endl;
            for(Number=1; Number<=9; Number++) // From number 1 to 9
            {
                PositionGood=false;       // set the position to false
                while(PositionGood==false)// While The Poisiton is bad
                {
                    PositionGood=true;      // Set the Position good to check if it's bad
                    rPositionX=1+(rand()%3);// Get random X
                    rPositionY=1+(rand()%3);// Get random Y
                    if (board.isEmpty(rPositionX, rPositionY))
                    {
                        MemoA=SPositionX*3+rPositionX; // Set Variable MemoA to remmember Position X in Sudoku
                        MemoB=SPositionY*3+rPositionY; // Set Variable MemoB to remmember Position Y in Sudoku
                        for(Counter=1; Counter<=9; Counter++) // From 1 to 9 | xxx-xxx-xxx and on y
                            if(Sudoku[MemoA][Counter]==Number || Sudoku[Counter][MemoB]==Number)  // Check Colon Y and then X while one of the two remains constant to see if it's clear of the same number
                            {
                                PositionGood=false;    // if it is set Position Bad
                            }
                    }
                    else
                    {
                        PositionGood=false;    // if Position in Container is taken , set Position bad
                    }
                    // So now if Position didn't go bad it's true and it will exit while
                }
                board.at(rPositionX, rPositionY) = Number;
            }
            for(CounterA=1; CounterA<=3; CounterA++)  // Counts the X Numbers in container
                for(CounterB=1; CounterB<=3; CounterB++) // Counts the Y Numbers in container
                {
                    Sudoku[SPositionX*3+CounterA][SPositionY*3+CounterB]=board.at(CounterA, CounterB);    // Some math to write the Container in Sudoku
                }
        }
    for(int i=1; i<=9; i++)
    {
        for(int y=1; y<=9; y++)
        {
            cout << Sudoku[i][y] << " ";    // Print all on Screen
        }
        cout << endl;
    }
    for(int i=1; i<=9; i++) // Print all on my TexT.txt file
    {
        for(int y=1; y<=9; y++)
        {
            if(y==3 || y==6)
            {
                Out << Sudoku[i][y] << "  ";
            }
            else
            {
                Out << board.at(i,y) << " ";
            }
        }
        if(i==3 || i==6)
        {
            Out << endl << endl;
        }
        else
        {
            Out << endl;
        }
    }
}
