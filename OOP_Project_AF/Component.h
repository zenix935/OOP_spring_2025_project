#pragma once
#include <string>
#include "Node.h"
using namespace std;
class Component
{
protected:
	string name;
	Node* node1;
	Node* node2;
public:
	Component(string name,Node* node1,Node* node2) : name(name),node1(node1),node2(node2) {}
	virtual double getCurrent()=0;
	virtual double getVoltage()=0;
	virtual double getSpecialValue()=0;
};

