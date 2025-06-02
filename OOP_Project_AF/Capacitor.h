#pragma once
#ifndef CAPACITOR_H
#define CAPACITOR_H
#include "Element.h"
#include "Complex.h"
class Capacitor : public Element 
{
private:
    double value; //Capacitance

public:
    Capacitor(const std::string& name,const std::string& node1,const std::string& node2,const std::string& valueStr);
    std::string toString() const override;
    double getValue() const { return value; }
    Complex getComplexAdmittance(double frequency) const override;
};
#endif