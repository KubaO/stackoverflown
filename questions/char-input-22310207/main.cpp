#include <QTextStream>
#include <QCoreApplication>

QTextStream cout(stdout);
QTextStream cin(stdin);

class GPSCoord {
   QString dir;
public:
   void setCoord(int, int, int, char d) { dir.append(d); }
   QString toString(bool) const { return dir; }
};

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    GPSCoord gps;
    int degrees, minutes, seconds;
    char cardinalDirection;

    cout << "\nPlease enter values for the latitude.." << endl;
    cout << "Degrees : " << flush;
    cin >> degrees;
    cout << "Minutes : " << flush;
    cin >> minutes;
    cout << "Seconds : " << flush;
    cin >> seconds;
    cout << "Cardinal Direction : " << flush;
    ws(cin) >> cardinalDirection;

    gps.setCoord(degrees, minutes, seconds, cardinalDirection);

    cout << "\nPlease enter values for the longitude.." << endl;
    cout << "Degrees : " << flush;
    cin >> degrees;
    cout << "Minutes : " << flush;
    cin >> minutes;
    cout << "Seconds : " << flush;
    cin >> seconds;
    cout << "Cardinal Direction : " << flush;
    ws(cin) >> cardinalDirection;

    gps.setCoord(degrees, minutes, seconds, cardinalDirection);

    cout << "\nGeographic Coordinates\t: " << gps.toString(false) << endl
         << "Decimal Coordinates\t: " << gps.toString(true) << endl;
    return 0;
}


