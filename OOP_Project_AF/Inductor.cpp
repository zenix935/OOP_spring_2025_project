#include "Inductor.h"
#include <string>
Inductor::Inductor(const std::string& name,const std::string& node1,const std::string& node2,const std::string& valueStr)
    : Element(name,node1,node2,"Inductor") 
{
    this->value=parseValue(valueStr);
    if(this->value<=0)
        throw InvalidValueError("Inductance cannot be zero or negative.");
}
std::string Inductor::toString() const { return "Inductor "+name+" "+node1+" "+node2+" "+std::to_string(value)+"H"; }
Complex Inductor::getComplexAdmittance(double frequency) const {
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