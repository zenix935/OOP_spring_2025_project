#ifndef RESISTOR_H
#define RESISTOR_H
#include "Element.h" 
#include "Complex.h"    
using namespace std;
class Resistor : public Element 
{ 
private:
    double value; // Resistance in Ohms 
public:
    Resistor(const string& name,const string& node1,const string& node2,const string& valueStr);

    // Override toString method
    string toString() const override;

    // Getter for value
    double getValue() const { return value; }

    // Override to get complex admittance for AC analysis
    Complex getComplexAdmittance(double frequency) const override;

    void stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
        const map<string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
        const vector<double>& prev_voltages,const vector<double>& prev_branch_currents) override;
};
#endif 