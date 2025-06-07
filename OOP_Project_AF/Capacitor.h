#ifndef CAPACITOR_H
#define CAPACITOR_H

#include "Element.h" // Changed from CircuitElement.h
#include "Complex.h"        // Include Complex number class

using namespace std;

class Capacitor : public Element { // Changed base class to Element
private:
    double value; // Capacitance in Farads
    double prev_voltage_diff; // V_node1 - V_node2 at previous time step
    double prev_current_through; // Current through capacitor at previous time step

public:
    // Constructor
    Capacitor(const string& name,const string& node1,const string& node2,const string& valueStr);

    // Override toString method
    string toString() const override;

    // Getter for value
    double getValue() const { return value; }

    // Override to get complex admittance for AC analysis
    Complex getComplexAdmittance(double frequency) const override;

    // Implement stampTransient for capacitors (Trapezoidal Rule)
    void stampTransient(Matrix<double>& A,vector<double>& b,
        const map<string,int>& nodeToIndex,
        const map<string,int>& voltageSourceNameToCurrentIndex,
        double dt,double time,
        const vector<double>& prev_voltages,
        const vector<double>& prev_branch_currents) override;

    // Override to update internal state after a transient step
    void updateTransientState(const vector<double>& current_voltages,
        const vector<double>& current_branch_currents,
        const map<string,int>& nodeToIndex,
        const map<string,int>& voltageSourceNameToCurrentIndex,
        double dt) override;

    // Override to initialize transient state
    void initializeTransientState() override;
};

#endif // CAPACITOR_H