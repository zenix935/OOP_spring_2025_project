#include "Resistor.h"
#include <string>
Resistor::Resistor(const string& name,const string& node1,const string& node2,const string& valueStr)
    : Element(name,node1,node2,"Resistor") 
{
    this->value=parseValue(valueStr);
    if(this->value<=0)
        throw InvalidValueError("Resistance cannot be zero or negative.");
}
// toString method implementation
string Resistor::toString() const { return "Resistor "+name+" "+node1+" "+node2+" "+to_string(value)+"?"; }
// Override to get complex admittance for AC analysis
Complex Resistor::getComplexAdmittance(double frequency) const { return Complex(1.0/value,0.0); }
void Resistor::stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
    const map<string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
    const vector<double>& prev_voltages,const vector<double>& prev_branch_currents) 
{
    (void)dt; // Unused in resistor stamping
    (void)time; // Unused in resistor stamping
    (void)prev_voltages; // Unused in resistor stamping
    (void)prev_branch_currents; // Unused in resistor stamping
    (void)voltageSourceNameToCurrentIndex; // Unused in resistor stamping
    double conductance=1.0/value;
    int idx1=(node1=="GND")?-1:nodeToIndex.at(node1);
    int idx2=(node2=="GND")?-1:nodeToIndex.at(node2);
    if(idx1!=-1)
        A.add(idx1,idx1,conductance);
    if(idx2!=-1)
        A.add(idx2,idx2,conductance);
    if(idx1!=-1&&idx2!=-1) 
    {
        A.add(idx1,idx2,-conductance);
        A.add(idx2,idx1,-conductance);
    }
}