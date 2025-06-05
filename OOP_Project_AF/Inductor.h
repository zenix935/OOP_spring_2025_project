#pragma once
#ifndef INDUCTOR_H
#define INDUCTOR_H
#include "Element.h"
#include "Complex.h"
class Inductor : public Element 
{
private:
    double value; //Inductance
    double prev_current; // Current through inductor at previous time step (from node1 to node2)
    double prev_voltage_across; // Voltage across inductor at previous time step (V(node1) - V(node2))
public:
    Inductor(const string& name,const string& node1,const string& node2,const string& valueStr);
    string toString() const override;
    double getValue() const { return value; }
    Complex getComplexAdmittance(double frequency) const override;
    void stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
        const map<string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
        const vector<double>& prev_voltages,const vector<double>& prev_branch_currents) override;
    void updateTransientState(const vector<double>& current_voltages,const vector<double>& current_branch_currents,
        const map<string,int>& nodeToIndex,const map<string,int>& voltageSourceNameToCurrentIndex) override;
    void initializeTransientState() override;
};
#endif