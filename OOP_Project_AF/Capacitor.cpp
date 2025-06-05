#include "Capacitor.h"
#include <string>
Capacitor::Capacitor(const std::string& name,const std::string& node1,const std::string& node2,const std::string& valueStr)
    : Element(name,node1,node2,"Capacitor"),prev_voltage_diff(0.0),prev_current_through(0.0)
{
    this->value=parseValue(valueStr);
    if(this->value<=0) 
        throw InvalidValueError("Capacitance cannot be zero or negative.");
}
// toString method implementation
std::string Capacitor::toString() const { return "Capacitor "+name+" "+node1+" "+node2+" "+std::to_string(value)+"F"; }
// Override to get complex admittance for AC analysis
Complex Capacitor::getComplexAdmittance(double frequency) const 
{
    // Admittance of a capacitor is j * omega * C
    double omega=2*M_PI*frequency;
    return J*Complex(0.0,omega*value);
}
// Implement stampTransient for capacitors (Trapezoidal Rule Companion Model)
void Capacitor::stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
    const map<string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
    const vector<double>& prev_voltages,const vector<double>& prev_branch_currents) 
{
    (void)time; // Unused in stamping
    (void)prev_branch_currents; // Unused for capacitor stamping
    (void)voltageSourceNameToCurrentIndex; // Unused for capacitor stamping
    if(dt<=0)
        throw CircuitError("Time step (dt) must be positive for transient analysis.");
    // Trapezoidal Rule Companion Model for Capacitor:
    // Current I_C(t) = G_eq * V_C(t) + I_eq
    // where G_eq = 2C / dt
    // and I_eq = (2C / dt) * V_C(t-dt) + I_C(t-dt)
    // We need V_C(t-dt) and I_C(t-dt) from previous step.
    double G_eq=2.0*value/dt; // Equivalent conductance
    // Equivalent current source (from previous state)
    double I_eq=G_eq*prev_voltage_diff+prev_current_through;
    int idx1=(node1=="GND")?-1:nodeToIndex.at(node1);
    int idx2=(node2=="GND")?-1:nodeToIndex.at(node2);
    // Stamp G_eq into the MNA matrix
    if(idx1!=-1)
        A.add(idx1,idx1,G_eq);
    if(idx2!=-1)
        A.add(idx2,idx2,G_eq);
    if(idx1!=-1&&idx2!=-1) 
    {
        A.add(idx1,idx2,-G_eq);
        A.add(idx2,idx1,-G_eq);
    }
    // Stamp I_eq into the RHS vector (current leaving node1, entering node2)
    if(idx1!=-1)
        b[idx1]-=I_eq;
    if(idx2!=-1)
        b[idx2]+=I_eq;
}
// Override to initialize transient state
void Capacitor::initializeTransientState() 
{
    prev_voltage_diff=0.0;
    prev_current_through=0.0;
}