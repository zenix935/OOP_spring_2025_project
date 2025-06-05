#pragma once
#ifndef ELEMENT_H
#define ELEMENT_H
#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <map>
#include "Exceptions.h"
#include "Complex.h"
#include "Matrix.h"
using namespace std;
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
	const string getName() const { return name; }
	const string getNode1() const { return node1; }
	const string getNode2() const { return node2; }
	const string getType() const { return type; }
	void setNode1(const string& newNode) { this->node1=newNode; }
	void setNode2(const string& newNode) { this->node2=newNode; }
	static double parseValue(const string& valueStr);
	virtual double getInstantaneousValue(double time) const { return 0.0; }
	virtual Complex getComplexAdmittance(double frequency) const { return Complex(0.0,0.0); }
	virtual void stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
		const map<string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
		const vector<double>& prev_voltages,const vector<double>& prev_branch_currents)=0;
	virtual void updateTransientState(const vector<double>& current_voltages,const vector<double>& current_branch_currents
		,const map<string,int>& nodeToIndex,const map<string,int>& voltageSourceNameToCurrentIndex){}
	virtual void initializeTransientState() {}
};
#endif 
