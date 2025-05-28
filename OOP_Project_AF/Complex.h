#pragma once
#ifndef COMPLEX_H
#define COMPLEX_H
#include <iostream>
#include <cmath>   
#include <iomanip> 
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
using namespace std;
class Complex
{
public:
	double real;
	double imag;
	Complex(double r=0.0,double i=0.0) : real(r),imag(i) {}
	Complex operator+(const Complex& other) const { return Complex(real+other.real,imag+other.imag); }
	Complex operator-(const Complex& other) const { return Complex(real-other.real,imag-other.imag); }
	Complex operator*(const Complex& other) const { return Complex(real*other.real-imag*other.imag,real*other.imag+imag*other.real); }
	Complex operator/(const Complex& other) const
	{
		double denominator=other.real*other.real+other.imag*other.imag;
		if(denominator==0)
		{
			throw runtime_error("Division by zero in complex division.");
			return Complex(numeric_limits<double>::infinity(),numeric_limits<double>::infinity());
		}
		return Complex((real*other.real+imag*other.imag)/denominator,(imag*other.real-real*other.imag)/denominator);
	}
	Complex operator+=(const Complex& other)
	{
		real+=other.real;
		imag+=other.imag;
		return *this;
	}
	Complex operator-=(const Complex& other)
	{
		real-=other.real;
		imag-=other.imag;
		return *this;
	}
	Complex operator-() const { return Complex(-real,-imag); }
	double mag() const { return sqrt(real*real+imag*imag); }
	double phaseRad() const { return atan2(imag,real); }
	double phaseDeg() const { return phaseRad()*180.0/M_PI; }
	Complex conj() const { return Complex(real,-imag); }
	static Complex polar(double magnitude,double phaseRad){ return Complex(magnitude*cos(phaseRad),magnitude*sin(phaseRad)); }
	friend ostream& operator<<(ostream& os,const Complex& c)
	{
		os<<fixed<<setprecision(4);
		os<<c.real;
		if(c.imag>=0)
			os<<" + "<<c.imag<<"j";
		else
			os<<" - "<<-c.imag<<"j";
		return os;
	}
};
const Complex J(0.0,1.0);
#endif
