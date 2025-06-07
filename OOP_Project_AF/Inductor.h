#ifndef INDUCTOR_H
#define INDUCTOR_H

#include "Element.h" // Include the base class
#include "Complex.h"        // Include Complex number class

class Inductor : public Element {
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
    void stampTransient(Matrix<double>& A,std::vector<double>& b,
        const std::map<std::string,int>& nodeToIndex,
        const std::map<std::string,int>& voltageSourceNameToCurrentIndex,
        double dt,double time,
        const std::vector<double>& prev_voltages,
        const std::vector<double>& prev_branch_currents) override;

    // Override to update internal state after a transient step
    // ADDED dt parameter
    void updateTransientState(const std::vector<double>& current_voltages,
        const std::vector<double>& current_branch_currents,
        const std::map<std::string,int>& nodeToIndex,
        const std::map<std::string,int>& voltageSourceNameToCurrentIndex,
        double dt) override; // ADDED dt

    // Override to initialize transient state
    void initializeTransientState() override;
};

#endif // INDUCTOR_H