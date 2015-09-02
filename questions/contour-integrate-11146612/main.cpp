#include <cmath>
#include <cassert>
#include <complex>
#include <iostream>

using std::cout;
using std::endl;
typedef std::complex<double> cplx;

typedef cplx (*function)(const cplx &);
typedef cplx (*path)(double);
typedef cplx (*rule)(function, const cplx &, const cplx &);

cplx inv(const cplx &z)
{
    return cplx(1,0)/z;
}

cplx unit_circle(double t)
{
    const double r = 1.0;
    const double c = 2*M_PI;
    return cplx(r*cos(c*t), r*sin(c*t));
}

cplx imag_exp_line_pi(double t)
{
    return exp(cplx(0, M_PI*t));
}

cplx trapezoid(function f, const cplx &a, const cplx &b)
{
    return 0.5 * (b-a) * (f(a)+f(b));
}

cplx integrate(function f, path path_, rule rule_)
{
    int counter = 0;
    double increment = .0001;
    cplx integral(0,0);
    cplx prev_point = path_(0.0);
    for (double i = increment; i <= 1.0; i += increment) {
        const cplx point = path_(i);
        integral += rule_(f, prev_point, point);
        prev_point = point;
        counter ++;
    }

    cout << "computed at " << counter << " values " << endl;
    cout << "the integral evaluates to " << integral << endl;
    return integral;
}

int main(int, char **)
{
    const double eps = 1E-7;
    cplx a, b;
    // Test in Octave using inverse and an exponential of a line
    // z = exp(1i*pi*(0:100)/100);
    // trapz(z, 1./z)
    // ans =
    //   0.0000 + 3.1411i
    a = integrate(inv, imag_exp_line_pi, trapezoid);
    b = cplx(0,M_PI);
    assert(abs(a-b) < eps*abs(b));

    // expected to be 2*PI*i
    a = integrate(inv, unit_circle, trapezoid);
    b = cplx(0,2*M_PI);
    assert(abs(a-b) < eps*abs(b));

    return 0;
}

