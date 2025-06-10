#ifndef INDUCTOR_H
#define INDUCTOR_H
#include "Element.h" 
#include "Complex.h"
class Inductor : public Element 
{
private:
    double value; // Inductance in Henries
    double prev_current; // Current through inductor at previous time step (from node1 to node2)
    double prev_voltage_across; // Voltage across inductor at previous time step (V(node1) - V(node2))
public:
    // Constructor
    Inductor(const std::string& name,const std::string& node1,const std::string& node2,const std::string& valueStr);

    // Override toString method
    std::string toString() const override;

    // Getter for value
    double getValue() const { return value; }

    // Override to get complex admittance for AC analysis
    Complex getComplexAdmittance(double frequency) const override;

    // Implement stampTransient for inductors (Trapezoidal Rule)
    void stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
        const map<std::string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
        const vector<double>& prev_voltages,const vector<double>& prev_branch_currents) override;

    // Override to update internal state after a transient step
    void updateTransientState(const vector<double>& current_voltages,const vector<double>& current_branch_currents,
        const map<string,int>& nodeToIndex,const map<string,int>& voltageSourceNameToCurrentIndex,double dt) override;

    // Override to initialize transient state
    void initializeTransientState() override;
};
#endif