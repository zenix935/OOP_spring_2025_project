#ifndef COMPLEX_H
#define COMPLEX_H

#include <cmath>   // For sqrt, atan2, cos, sin, M_PI
#include <iostream> // For ostream
#include <iomanip>  // For fixed, setprecision
#include <limits>   // For numeric_limits

// Define M_PI if it's not already defined (e.g., in some compilers, it's not by default)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

class Complex {
private:
    double real;
    double imag;

public:
    // Constructors
    Complex(double r=0.0,double i=0.0) : real(r),imag(i) {}

    // Getters
    double getReal() const { return real; }
    double getImag() const { return imag; }

    // Magnitude (Modulus)
    double magnitude() const {
        return sqrt(real*real+imag*imag);
    }

    // Phase Angle (in radians)
    double angleRadians() const {
        return atan2(imag,real);
    }

    // Phase Angle (in degrees)
    double angleDegrees() const {
        return angleRadians()*180.0/M_PI;
    }

    // Conjugate
    Complex conjugate() const {
        return Complex(real,-imag);
    }

    // Operator Overloads
    Complex operator+(const Complex& other) const {
        return Complex(real+other.real,imag+other.imag);
    }

    Complex operator-(const Complex& other) const {
        return Complex(real-other.real,imag-other.imag);
    }

    Complex operator*(const Complex& other) const {
        return Complex(real*other.real-imag*other.imag,
            real*other.imag+imag*other.real);
    }

    Complex operator/(const Complex& other) const {
        double denom=other.real*other.real+other.imag*other.imag;
        if(denom==0.0) {
            // Handle division by zero or very small numbers
            return Complex(numeric_limits<double>::infinity(),numeric_limits<double>::infinity());
        }
        return Complex((real*other.real+imag*other.imag)/denom,
            (imag*other.real-real*other.imag)/denom);
    }

    // Compound assignment operators
    Complex& operator+=(const Complex& other) {
        real+=other.real;
        imag+=other.imag;
        return *this;
    }

    Complex& operator-=(const Complex& other) {
        real-=other.real;
        imag-=other.imag;
        return *this;
    }

    Complex& operator*=(const Complex& other) {
        double newReal=real*other.real-imag*other.imag;
        double newImag=real*other.imag+imag*other.real;
        real=newReal;
        imag=newImag;
        return *this;
    }

    Complex& operator/=(const Complex& other) {
        double denom=other.real*other.real+other.imag*other.imag;
        if(denom==0.0) {
            // Handle division by zero
            real=numeric_limits<double>::infinity();
            imag=numeric_limits<double>::infinity();
            return *this;
        }
        double newReal=(real*other.real+imag*other.imag)/denom;
        double newImag=(imag*other.real-real*other.imag)/denom;
        real=newReal;
        imag=newImag;
        return *this;
    }

    // Unary minus
    Complex operator-() const {
        return Complex(-real,-imag);
    }

    // Comparison operators (useful for equality checks)
    bool operator==(const Complex& other) const {
        return real==other.real&&imag==other.imag;
    }

    bool operator!=(const Complex& other) const {
        return !(*this==other);
    }

    // Friend function for output stream
    friend ostream& operator<<(ostream& os,const Complex& c) {
        os<<fixed<<setprecision(4);
        if(c.imag==0) {
            os<<c.real;
        }
        else if(c.real==0) {
            os<<c.imag<<"j";
        }
        else {
            os<<c.real<<(c.imag>0?" + ":" - ")<<abs(c.imag)<<"j";
        }
        return os;
    }

    // Overloads for double * Complex, Complex * double
    friend Complex operator*(double val,const Complex& c) {
        return Complex(val*c.real,val*c.imag);
    }
    friend Complex operator*(const Complex& c,double val) {
        return Complex(val*c.real,val*c.imag);
    }

    // Overloads for double / Complex (1 / Complex)
    friend Complex operator/(double val,const Complex& c) {
        double denom=c.real*c.real+c.imag*c.imag;
        if(denom==0.0) {
            return Complex(numeric_limits<double>::infinity(),numeric_limits<double>::infinity());
        }
        return Complex(val*c.real/denom,-val*c.imag/denom);
    }
};

// Define the imaginary unit 'J' directly as a Complex number
const Complex J(0.0,1.0);

// Global abs overload for Complex numbers
inline double abs(const Complex& c) {
    return c.magnitude();
}

#endif // COMPLEX_H