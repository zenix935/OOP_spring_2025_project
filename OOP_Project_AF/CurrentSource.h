#ifndef CURRENTSOURCE_H
#define CURRENTSOURCE_H
#include "Element.h" 
#include "Complex.h"     
#include <string>          
class CurrentSource : public Element 
{
private:
    double dc_value;     // DC current in Amps
    double ac_magnitude; // AC magnitude in Amps
    double ac_phase;     // AC phase in degrees
    double ac_frequency; // AC frequency in Hz (0 for DC source, or if not an AC source)

    // Helper to check if it's an AC source
    bool isACSource() const { return ac_magnitude>0||ac_phase!=0||ac_frequency>0; }
public:
    // Constructor for DC Current Source
    CurrentSource(const string& name,const string& node1,const string& node2,const string& dcValueStr);

    // Constructor for AC Current Source (magnitude, phase, frequency)
    CurrentSource(const string& name,const string& node1,const string& node2,
        const string& acMagnitudeStr,const string& acPhaseStr,const string& acFrequencyStr);

    // Override toString method
    std::string toString() const override;

    // Getters for DC/AC values
    double getDCValue() const { return dc_value; }
    void setDCValue(double new_val) { dc_value=new_val; } // For DC sweep

    // Get the phasor for AC analysis at a given frequency
    Complex getACPhasor(double analysisFrequency) const;

    // Get instantaneous value for transient analysis
    double getInstantaneousValue(double time) const override;

    // Current sources do not have admittance for MNA (they are treated as current injection)
    Complex getComplexAdmittance(double frequency) const override { return Complex(0.0,0.0); } // No direct admittance for current sources
    void stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
        const map<string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
        const vector<double>& prev_voltages,const vector<double>& prev_branch_currents) override; // No operation for CurrentSource
};
#endif 