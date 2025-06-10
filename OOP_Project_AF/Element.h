#ifndef ELEMENT_H
#define ELEMENT_H
#include <string>
#include <algorithm> 
#include <stdexcept> 
#include <vector>   
#include <cctype>    
#include <map>       
#include "Exceptions.h" 
#include "Complex.h"    
using namespace std;
// Forward declaration of Matrix for stampTransient, as it's a templated class.
template <typename T> class Matrix;
// Function to convert string to uppercase
inline string toUpper(string s) 
{
    transform(s.begin(),s.end(),s.begin(),[](unsigned char c){ return toupper(c); });
    return s;
}

// Base class for all circuit elements
class Element 
{
protected:
    string name;
    string node1;
    string node2;
    string type;
public:
    // Constructor
    Element(const string& name,const string& node1,const string& node2,const string& type);

    // Virtual destructor to ensure proper cleanup of derived classes
    virtual ~Element()=default; // Renamed destructor

    // Pure virtual function for string representation (must be implemented by derived classes)
    virtual string toString() const=0;

    // Getters
    const string& getName() const { return name; }
    const string& getNode1() const { return node1; }
    const string& getNode2() const { return node2; }
    const string& getType() const { return type; }

    // Setters for nodes (needed for node renaming)
    void setNode1(const string& newNode) { this->node1=toUpper(newNode); }
    void setNode2(const string& newNode) { this->node2=toUpper(newNode); }

    // Static helper to parse value with units (e.g., 1k, 1u, 1m, 1Meg)
    static double parseValue(const string& valueStr);

    // Virtual method to get instantaneous value for time-varying sources (for transient analysis)
    // Returns DC value for non-source elements or DC sources, time-varying value for AC sources.
    virtual double getInstantaneousValue(double time) const { return 0.0; }

    // Pure virtual method to get complex admittance for AC analysis
    virtual Complex getComplexAdmittance(double frequency) const { return Complex(0.0,0.0); } // Default for elements that don't have a simple admittance

    // Pure virtual method to stamp contributions for transient analysis (Trapezoidal Rule Companion Model)
    // This will be called at each time step to build the MNA matrix and RHS vector.
    // prev_voltages: vector of node voltages from the previous time step.
    // prev_branch_currents: vector of branch currents (for V sources, L) from the previous time step.
    virtual void stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
        const map<string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
        const vector<double>& prev_voltages,const vector<double>& prev_branch_currents)=0;

    // Virtual method to update the internal state of the element after a transient step solution
    // Default implementation does nothing for elements without state (R, sources).
    // Capacitors and Inductors will override this.
    virtual void updateTransientState(const vector<double>& current_voltages,const vector<double>& current_branch_currents,
        const map<string,int>& nodeToIndex,const map<string,int>& voltageSourceNameToCurrentIndex,double dt) {}

    // Virtual method to initialize transient state (e.g., initial capacitor voltage, inductor current)
    virtual void initializeTransientState() {}
};
#endif