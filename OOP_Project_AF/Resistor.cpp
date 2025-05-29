#include "Resistor.h"
#include <string>
Resistor::Resistor(const std::string& name,const std::string& node1,const std::string& node2,const std::string& valueStr)
    : Element(name,node1,node2,"Resistor") 
{
    this->value=parseValue(valueStr);
    if(this->value<=0)
        throw InvalidValueError("Resistance cannot be zero or negative.");
}
// toString method implementation
std::string Resistor::toString() const { return "Resistor "+name+" "+node1+" "+node2+" "+std::to_string(value)+"?"; }
// Override to get complex admittance for AC analysis
Complex Resistor::getComplexAdmittance(double frequency) const { return Complex(1.0/value,0.0); }