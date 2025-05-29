#pragma once
#ifndef RESISTOR_H
#define RESISTOR_H
#include "Element.h"
#include "Complex.h"
class Resistor : public Element 
{
private:
    double value; //Resistance
public:
    Resistor(const std::string& name,const std::string& node1,const std::string& node2,const std::string& valueStr);
    std::string toString() const override;
    double getValue() const { return value; }
    Complex getComplexAdmittance(double frequency) const override;
};
#endif 