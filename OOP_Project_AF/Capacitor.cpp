#include "Capacitor.h"
#include <string>
Capacitor::Capacitor(const std::string& name,const std::string& node1,const std::string& node2,const std::string& valueStr)
    : Element(name,node1,node2,"Capacitor") 
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