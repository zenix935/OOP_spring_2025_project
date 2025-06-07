#include "Resistor.h"
#include <string> // For to_string

using namespace std;

// Constructor implementation
Resistor::Resistor(const string& name,const string& node1,const string& node2,const string& valueStr)
    : Element(name,node1,node2,"Resistor") { // Changed base class constructor call
    this->value=parseValue(valueStr);
    if(this->value<=0) { // Resistance should be positive
        throw InvalidValueError("Resistance cannot be zero or negative.");
    }
}

// toString method implementation
string Resistor::toString() const {
    return "Resistor "+name+" "+node1+" "+node2+" "+to_string(value)+"Ohm";
}

// Override to get complex admittance for AC analysis
Complex Resistor::getComplexAdmittance(double frequency) const {
    // Admittance of a resistor is simply 1/R (real part only)
    return Complex(1.0/value,0.0);
}

// Resistors do not have a time-dependent companion model for transient analysis
// beyond their simple conductance, which is handled directly in Circuit::solveTransient.
// This method is intentionally empty to acknowledge it's not needed for this component.
void Resistor::stampTransient(Matrix<double>& A,vector<double>& b,
    const map<string,int>& nodeToIndex,
    const map<string,int>& voltageSourceNameToCurrentIndex,
    double dt,double time,
    const vector<double>& prev_voltages,
    const vector<double>& prev_branch_currents) {
    // Resistors are handled by their direct conductance stamping in Circuit::solveTransient.
    // The parameters are marked (void) to suppress unused variable warnings.
    (void)A; (void)b; (void)nodeToIndex; (void)voltageSourceNameToCurrentIndex; (void)dt; (void)time; (void)prev_voltages; (void)prev_branch_currents;
}