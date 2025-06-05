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
    void stampTransient(Matrix<double>& A,std::vector<double>& b,
        const std::map<std::string,int>& nodeToIndex,
        const std::map<std::string,int>& voltageSourceNameToCurrentIndex,
        double dt,double time,
        const std::vector<double>& prev_voltages,
        const std::vector<double>& prev_branch_currents) override;
};
#endif 