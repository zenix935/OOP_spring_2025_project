#include "Inductor.h"
#include <string>
Inductor::Inductor(const std::string& name,const std::string& node1,const std::string& node2,const std::string& valueStr)
    : Element(name,node1,node2,"Inductor"),prev_current(0.0),prev_voltage_across(0.0)
{
    this->value=parseValue(valueStr);
    if(this->value<=0)
        throw InvalidValueError("Inductance cannot be zero or negative.");
}
std::string Inductor::toString() const { return "Inductor "+name+" "+node1+" "+node2+" "+std::to_string(value)+"H"; }
Complex Inductor::getComplexAdmittance(double frequency) const 
{
    // Admittance of an inductor is 1 / (j * omega * L) = -j / (omega * L)
    double omega=2*M_PI*frequency;
    if(omega==0) 
    {   // Handle DC case (short circuit)
        // For AC analysis at DC (f=0), inductor is a short. Admittance is infinite.
        // This can cause issues. For MNA, it means V_n1 - V_n2 = 0.
        // A common way to handle this in MNA is to add a branch current and a constraint.
        // For now, returning 0, but this needs careful handling in MNA.
        return Complex(0.0,0.0); // Effectively an open circuit for admittance, but MNA needs short.
    }
    return Complex(0.0,-1.0/(omega*value));
}
// Implement stampTransient for inductors (Trapezoidal Rule Companion Model)
void Inductor::stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
    const map<string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
    const vector<double>& prev_voltages,const vector<double>& prev_branch_currents) 
{
    (void)time; // Unused in stamping
    (void)prev_voltages; // Unused for inductor stamping directly into MNA
    (void)voltageSourceNameToCurrentIndex; // Unused for inductor stamping
    if(dt<=0)
        throw CircuitError("Time step (dt) must be positive for transient analysis.");
    // Inductors add a branch current variable to the MNA matrix.
    // The branch current index for this inductor is needed.
    // For now, assuming inductors are treated like voltage sources in terms of adding branch currents
    // to the MNA formulation. This is a simplification. A more robust MNA would have a separate
    // mapping for inductor currents.
    // For this implementation, we'll assume the inductor's current is added as a new unknown.
    // This requires the Circuit class to manage inductor branch current indices.
    // For now, I'll use a placeholder for branchIdx and note this needs proper handling.
    // This is a critical point: Inductors *do* add branch currents in MNA, similar to voltage sources.
    // The Circuit class needs to manage these.
    // Let's assume for now that the Circuit class will pass the correct branch index for this inductor.
    // To properly stamp an inductor with Trapezoidal Rule, it's often treated as a voltage source
    // with a series equivalent resistance and an equivalent voltage source.
    // V_L(t) = R_eq * I_L(t) + V_eq
    // where R_eq = 2L / dt
    // and V_eq = (2L / dt) * I_L(t-dt) + V_L(t-dt)
    // This requires adding a branch current for the inductor.
    // Since the current MNA structure only explicitly maps voltage sources to branch currents,
    // we need to extend that. For this implementation, I'll assume inductors also get a unique
    // branch current index in the MNA matrix, similar to voltage sources.
    // This means `voltageSourceNameToCurrentIndex` should ideally be `elementNameToBranchCurrentIndex`.
    // For simplicity, let's assume `voltageSourceNameToCurrentIndex` *also* contains inductor names.
    // This is a hacky workaround for the current structure. A proper MNA would differentiate.
    // Or, more correctly, inductors are typically handled by adding a new row/column for their current.
    // Let's assume the inductor's current is the (num_nodes + inductor_index) variable.
    // This requires the Circuit to manage these indices.
    // Given the current structure, the simplest way to integrate L is to treat it as a voltage source
    // with a time-dependent value and series resistance for transient analysis.
    // This means we need to add a branch current for it.
    // For now, let's assume the inductor's current is implicitly handled by the MNA structure
    // if it's treated as a short circuit in DC. For transient, it's a series R-V_eq.
    // This means it *does* add a branch current.
    // Let's re-evaluate the MNA size and branch current indexing in Circuit.cpp
    // to include inductors in the branch current count.
    // For the companion model:
    // V_n1 - V_n2 - L_branch_current_variable * (2L/dt) = (2L/dt) * I_L(t-dt) + V_L(t-dt)
    // KCL at n1: ... + I_L_branch_current_variable = 0
    // KCL at n2: ... - I_L_branch_current_variable = 0
    // This implies that Inductors also need to be added to the `voltageSourceNameToCurrentIndex` map
    // (or a new `inductorNameToCurrentIndex` map). Let's modify `Circuit` to include them.
    // For now, let's assume the index for this inductor's current is available.
    // If it's not, the current `voltageSourceNameToCurrentIndex` map will throw.
    auto it=voltageSourceNameToCurrentIndex.find(name);
    if(it==voltageSourceNameToCurrentIndex.end())
        throw CircuitError("Inductor '"+name+"' does not have a registered branch current index for transient analysis.");
    int branchIdx=it->second;
    double R_eq=2.0*value/dt; // Equivalent resistance
    double V_eq=R_eq*prev_current+prev_voltage_across; // Equivalent voltage source
    int idx1=(node1=="GND")?-1:nodeToIndex.at(node1);
    int idx2=(node2=="GND")?-1:nodeToIndex.at(node2);
    // KCL equations (rows corresponding to nodes)
    if(idx1!=-1)
        A.add(idx1,branchIdx,1.0); // +1 coefficient for branch current leaving n1
    if(idx2!=-1)
        A.add(idx2,branchIdx,-1.0); // -1 coefficient for branch current entering n2
    // Voltage constraint equation (row corresponding to branch current)
    // V_n1 - V_n2 - I_L * R_eq = V_eq
    if(idx1!=-1)
        A.add(branchIdx,idx1,1.0);
    if(idx2!=-1)
        A.add(branchIdx,idx2,-1.0);
    A.add(branchIdx,branchIdx,-R_eq); // Coefficient for inductor current variable
    b[branchIdx]+=V_eq; // RHS for voltage constraint
}
// Override to update internal state after a transient step
void Inductor::updateTransientState(const vector<double>& current_voltages,const vector<double>& current_branch_currents,
    const map<string,int>& nodeToIndex,const map<string,int>& voltageSourceNameToCurrentIndex) 
{
    (void)nodeToIndex; // Not directly used for inductor state update
    auto it=voltageSourceNameToCurrentIndex.find(name);
    if(it==voltageSourceNameToCurrentIndex.end()) 
    {
        // This should not happen if stampTransient succeeded.
        std::cerr<<"Error: Inductor '"<<name<<"' missing branch current index during state update.\n";
        return;
    }
    int branchIdx=it->second;
    // The current through the inductor is the solved branch current
    prev_current=current_branch_currents[branchIdx];
    // The voltage across the inductor at the current step (for next step's V_eq)
    int idx1=(node1=="GND")?-1:nodeToIndex.at(node1);
    int idx2=(node2=="GND")?-1:nodeToIndex.at(node2);
    double V1=(idx1!=-1)?current_voltages[idx1]:0.0;
    double V2=(idx2!=-1)?current_voltages[idx2]:0.0;
    prev_voltage_across=V1-V2;
}
// Override to initialize transient state
void Inductor::initializeTransientState() 
{
    prev_current=0.0;
    prev_voltage_across=0.0;
}