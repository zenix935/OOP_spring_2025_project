#ifndef RESISTOR_H
#define RESISTOR_H

#include "Element.h" // Changed from CircuitElement.h
#include "Complex.h"        // For AC analysis

using namespace std;

class Resistor : public Element { // Changed base class to Element
private:
    double value; // Resistance in Ohms

public:
    // Constructor
    Resistor(const string& name,const string& node1,const string& node2,const string& valueStr);

    // Override toString method
    string toString() const override;

    // Getter for value
    double getValue() const { return value; }

    // Override to get complex admittance for AC analysis
    Complex getComplexAdmittance(double frequency) const override;

    // Resistors do not have a time-dependent companion model for transient analysis
    // beyond their simple conductance, which is handled directly in Circuit::solveTransient.
    // This method is empty to acknowledge it's not needed for this component.
    void stampTransient(Matrix<double>& A,vector<double>& b,
        const map<string,int>& nodeToIndex,
        const map<string,int>& voltageSourceNameToCurrentIndex,
        double dt,double time,
        const vector<double>& prev_voltages,
        const vector<double>& prev_branch_currents) override; // No operation for Resistor
};

#endif // RESISTOR_H