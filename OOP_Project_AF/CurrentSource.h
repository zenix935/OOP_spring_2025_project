#ifndef CURRENTSOURCE_H
#define CURRENTSOURCE_H

#include "Element.h" // Include the base class
#include "Complex.h"        // For AC analysis
#include <string>           // For std::string

class CurrentSource : public Element {
private:
    double dc_value;     // DC current in Amps
    double ac_magnitude; // AC magnitude in Amps
    double ac_phase;     // AC phase in degrees
    double ac_frequency; // AC frequency in Hz (0 for DC source, or if not an AC source)

    // Helper to check if it's an AC source
    bool isACSource() const { return ac_magnitude>0||ac_phase!=0||ac_frequency>0; }

public:
    // Constructor for DC Current Source
    CurrentSource(const std::string& name,const std::string& node1,const std::string& node2,const std::string& dcValueStr);

    // Constructor for AC Current Source (magnitude, phase, frequency)
    CurrentSource(const std::string& name,const std::string& node1,const std::string& node2,
        const std::string& acMagnitudeStr,const std::string& acPhaseStr,const std::string& acFrequencyStr);

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
    Complex getComplexAdmittance(double frequency) const override {
        return Complex(0.0,0.0); // No direct admittance for current sources
    }

    // Current sources do not typically "stamp" in the traditional sense,
    // they contribute to the RHS vector. But if we decide to model them
    // as conductances at all (e.g. for Norton equivalent), this is where it would be.
    // For MNA, independent current sources are on the RHS.
    void stampTransient(Matrix<double>& A,std::vector<double>& b,
        const std::map<std::string,int>& nodeToIndex,
        const std::map<std::string,int>& voltageSourceNameToCurrentIndex,
        double dt,double time,
        const std::vector<double>& prev_voltages,
        const std::vector<double>& prev_branch_currents) override; // No operation for CurrentSource
};

#endif // CURRENTSOURCE_H