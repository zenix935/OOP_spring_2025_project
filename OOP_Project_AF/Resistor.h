#pragma once
#include "Component.h"
class Resistor : public Component
{
private:
	double resistance;
public:
	Resistor(double resistance,string name,Node* node1,Node* node2) : Component(name,node1,node2),resistance(resistance) {}
	double getVoltage() override{return node1->getVoltage()-node2->getVoltage();}
	double getCurrent() override{ return node1->getVoltage()-node2->getVoltage()/resistance; }
	double getSpecialValue() override{ return resistance; }
};

