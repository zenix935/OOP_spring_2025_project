#ifndef VOLTAGESOURCE_H
#define VOLTAGESOURCE_H

#include "Element.h" // Changed from CircuitElement.h
#include "Complex.h"        // For AC analysis
#include <string>           // For string

using namespace std;

class VoltageSource : public Element { // Changed base class to Element
private:
    double dc_value;     // DC voltage in Volts
    double ac_magnitude; // AC magnitude in Volts
    double ac_phase;     // AC phase in degrees
    double ac_frequency; // AC frequency in Hz (0 for DC source, or if not an AC source)

    // Helper to check if it's an AC source
    bool isACSource() const { return ac_magnitude>0||ac_phase!=0||ac_frequency>0; }

public:
    // Constructor for DC Voltage Source
    VoltageSource(const string& name,const string& node1,const string& node2,const string& dcValueStr);

    // Constructor for AC Voltage Source (magnitude, phase, frequency)
    VoltageSource(const string& name,const string& node1,const string& node2,
        const string& acMagnitudeStr,const string& acPhaseStr,const string& acFrequencyStr);

    // Override toString method
    string toString() const override;

    // Getters for DC/AC values
    double getDCValue() const { return dc_value; }
    void setDCValue(double new_val) { dc_value=new_val; } // For DC sweep

    // Get the phasor for AC analysis at a given frequency
    Complex getACPhasor(double analysisFrequency) const;

    // Get instantaneous value for transient analysis
    double getInstantaneousValue(double time) const override;

    // Voltage sources do not have admittance for MNA (they add a row/column for branch current)
    Complex getComplexAdmittance(double frequency) const override {
        // A voltage source is ideally a short circuit with a controlled voltage.
        // Its admittance is theoretically infinite, but for MNA, it's handled
        // by adding a branch current variable, not by stamping an admittance.
        return Complex(0.0,0.0);
    }

    // Independent Voltage Sources add a branch current variable.
    // The stampTransient method handles their contribution to the MNA matrix and RHS.
    void stampTransient(Matrix<double>& A,vector<double>& b,
        const map<string,int>& nodeToIndex,
        const map<string,int>& voltageSourceNameToCurrentIndex,
        double dt,double time,
        const vector<double>& prev_voltages,
        const vector<double>& prev_branch_currents) override; // Handled directly in Circuit::solveTransient
};

#endif // VOLTAGESOURCE_H