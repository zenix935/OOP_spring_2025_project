#pragma once
#ifndef CAPACITOR_H
#define CAPACITOR_H
#include "Element.h"
#include "Complex.h"
class Capacitor : public Element 
{
private:
    double value; //Capacitance
    double prev_voltage_diff; // V(node1) - V(node2) at previous time step
    double prev_current_through; // Current flowing from node1 to node2 at previous time step

public:
    Capacitor(const std::string& name,const std::string& node1,const std::string& node2,const std::string& valueStr);
    std::string toString() const override;
    double getValue() const { return value; }
    Complex getComplexAdmittance(double frequency) const override;
    void stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
        const map<string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
        const std::vector<double>& prev_voltages,const std::vector<double>& prev_branch_currents) override;
    void updateTransientState(const vector<double>& current_voltages,const vector<double>& current_branch_currents,
        const map<string,int>& nodeToIndex,const map<string,int>& voltageSourceNameToCurrentIndex) override;
    void initializeTransientState() override;
};
#endif