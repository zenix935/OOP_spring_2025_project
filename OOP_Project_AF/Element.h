#pragma once
#ifndef ELEMENT_H
#define ELEMENT_H
#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include "Exceptions.h"
#include "Complex.h"
using namespace std;
inline string toUpper(string str) 
{
	transform(str.begin(),str.end(),str.begin(),::toupper);
	return str;
}
class Element
{
protected:
	string name;
	string node1;
	string node2;
	string type;
public:
	Element(const string& name,const string& node1,const string& node2,const string& type);
	virtual ~Element()=default;
	virtual string toString() const=0;
	string getName() const { return name; }
	string getNode1() const { return node1; }
	string getNode2() const { return node2; }
	string getType() const { return type; }
	void setNode1(const string& newNode) { this->node1=newNode; }
	void setNode2(const string& newNode) { this->node2=newNode; }
	static double parseValue(const string& valueStr);
	virtual Complex getComplexAdmittance(double frequency) const { return Complex(0.0,0.0); }
};
#endif 
