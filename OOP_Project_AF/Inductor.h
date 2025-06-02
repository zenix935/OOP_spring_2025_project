#pragma once
#ifndef INDUCTOR_H
#define INDUCTOR_H
#include "Element.h"
#include "Complex.h"
class Inductor : public Element 
{
private:
    double value; //Inductance
public:
    Inductor(const std::string& name,const std::string& node1,const std::string& node2,const std::string& valueStr);
    std::string toString() const override;
    double getValue() const { return value; }
    Complex getComplexAdmittance(double frequency) const override;
};
#endif