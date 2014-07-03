#include <QApplication>
#include <QDateTime>
#include <QDate>
#include <QDebug>
#include <cmath>

static const double pi = 3.14159265358979323846;

class SunData {
   // Based on http://www.esrl.noaa.gov/gmd/grad/solcalc/calcdetails.html
   QDateTime m_reference;
   QTime m_dawn;
   QTime m_sunrise;
   QTime m_noon;
   QTime m_sunset;
   QTime m_dusk;
   typedef double T;
   T RADIANS(T deg) { return deg*pi/180.; }
   T DEGREES(T rad) { return rad*180./pi; }
   T MOD(T a, T b) { return fmod(a, b); }
   T SIN(T x) { return sin(x); }
   T COS(T x) { return cos(x); }
   T TAN(T x) { return tan(x); }
   T ASIN(T x) { return asin(x); }
   T ACOS(T x) { return acos(x); }
   T ATAN2(T a, T b) { return atan2(a, b); }
   T days2msecs(T days) { return days * 24.0 * 3600.0 * 1E3; }
   void calc(QDateTime reference, T latitude /* + to North */, T longitude /* + to East */) {
      T $B$5 = reference.offsetFromUtc()/3600.; // TZ offset, [hr] + to East
      T $B$3 = latitude;
      T $B$4 = longitude;
      T F2 = reference.date().toJulianDay() - $B$5/24.0;
      T G2 = (F2-2451545.)/36525.;
      T I2 = MOD(280.46646+G2*(36000.76983+G2*0.0003032),360);
      T J2 = 357.52911+G2*(35999.05029-0.0001537*G2);
      T K2 = 0.016708634-G2*(0.000042037+0.0000001267*G2);
      T L2 = SIN(RADIANS(J2))*(1.914602-G2*(0.004817+0.000014*G2))+
            SIN(RADIANS(2*J2))*(0.019993-0.000101*G2)+SIN(RADIANS(3*J2))*0.000289;
      T M2 = I2+L2;
      T P2 = M2-0.00569-0.00478*SIN(RADIANS(125.04-1934.136*G2));
      T Q2 = 23.+(26+((21.448-G2*(46.815+G2*(0.00059-G2*0.001813))))/60.)/60.;
      T R2 = Q2+0.00256*COS(RADIANS(125.04-1934.136*G2));
      T T2 = DEGREES(ASIN(SIN(RADIANS(R2))*SIN(RADIANS(P2))));
      T U2 = TAN(RADIANS(R2/2))*TAN(RADIANS(R2/2));
      T V2 = 4.*DEGREES(U2*SIN(2*RADIANS(I2))-2*K2*SIN(RADIANS(J2))
                       +4.*K2*U2*SIN(RADIANS(J2))*COS(2*RADIANS(I2))
                       -0.5*U2*U2*SIN(4*RADIANS(I2))-1.25*K2*K2*SIN(2*RADIANS(J2)));
      T W2 = DEGREES(ACOS(COS(RADIANS(90.833))/(COS(RADIANS($B$3))*COS(RADIANS(T2)))
                          -TAN(RADIANS($B$3))*TAN(RADIANS(T2))));
      T X2 = (720.-4.*$B$4-V2+$B$5*60.)/1440.; // solar noon (LST, in days)
      T Y2 = X2-W2*4./1440.; // sunrise (LST, in days)
      T Z2 = X2+W2*4./1440.; // sunset (LST, in days)
      T dawn = X2-(W2+30.)*4./1440.; // astronomical dawn (LST, in days)
      T dusk = X2+(W2+30.)*4./1440.; // astronomical dusk (LST, in days)
      m_dawn = QTime::fromMSecsSinceStartOfDay(days2msecs(dawn));
      m_sunrise = QTime::fromMSecsSinceStartOfDay(days2msecs(Y2));
      m_noon = QTime::fromMSecsSinceStartOfDay(days2msecs(X2));
      m_sunset = QTime::fromMSecsSinceStartOfDay(days2msecs(Z2));
      m_dusk = QTime::fromMSecsSinceStartOfDay(days2msecs(dusk));
   }

public:
   explicit SunData(const QDateTime & reference, qreal latitude, qreal longitude)
      : m_reference(reference)
   {
      m_reference.setTime(QTime(0,0));
      calc(m_reference, latitude, longitude);
   }
   const QTime & dawn() const { return m_dawn; }
   const QTime & sunrise() const { return m_sunrise; }
   const QTime & noon() const { return m_noon; }
   const QTime & sunset() const { return m_sunset; }
   const QTime & dusk() const { return m_dusk; }
};

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   QMetaObject::invokeMethod(&a, "quit", Qt::QueuedConnection);

   SunData sd(QDateTime::currentDateTime(), 39.9833, -82.9833);
   qDebug() << sd.dawn() << sd.sunrise() << sd.noon() << sd.sunset() << sd.dusk();
   return a.exec();
}
