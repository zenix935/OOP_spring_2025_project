
#include "Inductor.h"
#include "Matrix.h"   // <--- ADDED THIS LINE
#include <string>     // For to_string
#include <cmath>      // For M_PI

using namespace std;

// Constructor implementation
Inductor::Inductor(const string& name,const string& node1,const string& node2,const string& valueStr)
    : Element(name,node1,node2,"Inductor"),prev_current(0.0),prev_voltage_across(0.0) {
    this->value=parseValue(valueStr);
    if(this->value<=0) {
        throw InvalidValueError("Inductance cannot be zero or negative.");
    }
}

// toString method implementation
string Inductor::toString() const {
    return "Inductor "+name+" "+node1+" "+node2+" "+to_string(value)+"H";
}

// Override to get complex admittance for AC analysis
Complex Inductor::getComplexAdmittance(double frequency) const {
    // Admittance of an inductor is 1 / (j * omega * L) = -j / (omega * L)
    double omega=2*M_PI*frequency;
    if(omega==0) { // Handle DC case (short circuit)
        return Complex(0.0,0.0); // Treated as open for admittance, but MNA needs short.
    }
    return Complex(0.0,-1.0/(omega*value));
}

// Implement stampTransient for inductors (Trapezoidal Rule Companion Model)
void Inductor::stampTransient(Matrix<double>& A,vector<double>& b,
    const map<string,int>& nodeToIndex,
    const map<string,int>& voltageSourceNameToCurrentIndex,
    double dt,double time,
    const vector<double>& prev_voltages,
    const vector<double>& prev_branch_currents) {
    (void)time; // Unused in stamping
    (void)prev_voltages; // Unused for inductor stamping directly into MNA
    (void)voltageSourceNameToCurrentIndex; // Unused for inductor stamping

    if(dt<=0) {
        throw CircuitError("Time step (dt) must be positive for transient analysis.");
    }

    auto it=voltageSourceNameToCurrentIndex.find(name);
    if(it==voltageSourceNameToCurrentIndex.end()) {
        throw CircuitError("Inductor '"+name+"' does not have a registered branch current index for transient analysis.");
    }
    int branchIdx=it->second;

    double R_eq=2.0*value/dt; // Equivalent resistance
    double V_eq=R_eq*prev_current+prev_voltage_across; // Equivalent voltage source

    int idx1=(node1=="GND")?-1:nodeToIndex.at(node1);
    int idx2=(node2=="GND")?-1:nodeToIndex.at(node2);

    // KCL equations (rows corresponding to nodes)
    if(idx1!=-1) {
        A.add(idx1,branchIdx,1.0); // +1 coefficient for branch current leaving n1
    }
    if(idx2!=-1) {
        A.add(idx2,branchIdx,-1.0); // -1 coefficient for branch current entering n2
    }

    // Voltage constraint equation (row corresponding to branch current)
    // V_n1 - V_n2 - I_L * R_eq = V_eq
    if(idx1!=-1) {
        A.add(branchIdx,idx1,1.0);
    }
    if(idx2!=-1) {
        A.add(branchIdx,idx2,-1.0);
    }
    A.add(branchIdx,branchIdx,-R_eq); // Coefficient for inductor current variable
    b[branchIdx]+=V_eq; // RHS for voltage constraint
}

// Override to update internal state after a transient step
void Inductor::updateTransientState(const vector<double>& current_voltages,
    const vector<double>& current_branch_currents,
    const map<string,int>& nodeToIndex,
    const map<string,int>& voltageSourceNameToCurrentIndex,
    double dt) {
    (void)nodeToIndex; // Not directly used for inductor state update
    (void)dt; // dt is needed for the calculation

    auto it=voltageSourceNameToCurrentIndex.find(name);
    if(it==voltageSourceNameToCurrentIndex.end()) {
        // This should not happen if stampTransient succeeded.
        cerr<<"Error: Inductor '"<<name<<"' missing branch current index during state update.\n";
        return;
    }
    int branchIdx=it->second;

    // The current through the inductor is the solved branch current
    double current_I=current_branch_currents[branchIdx];

    // The voltage across the inductor at the current step (for next step's V_eq)
    int idx1=(node1=="GND")?-1:nodeToIndex.at(node1);
    int idx2=(node2=="GND")?-1:nodeToIndex.at(node2);
    double V1=(idx1!=-1)?current_voltages[idx1]:0.0;
    double V2=(idx2!=-1)?current_voltages[idx2]:0.0;
    double current_V_across=V1-V2;

    // Update previous values
    prev_current=current_I;
    prev_voltage_across=current_V_across;
}

// Override to initialize transient state
void Inductor::initializeTransientState() {
    prev_current=0.0;
    prev_voltage_across=0.0;
}
