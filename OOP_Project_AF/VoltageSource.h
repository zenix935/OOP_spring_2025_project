#pragma once
#ifndef VOLTAGE_SOURCE_H
#define VOLTAGE_SOURCE_H
#include "Element.h"
#include "Complex.h"
class VoltageSource : public Element 
{
private:
    double dc_value; // DC voltage value in Volts
    // For AC analysis, voltage sources can have magnitude and phase
    double ac_magnitude;
    double ac_phase_deg; // Phase in degrees
    double ac_frequency; // Frequency in Hz for AC sources (for transient analysis)
public:
    // Constructor for DC analysis
    VoltageSource(const string& name,const string& node1,const string& node2,const string& valueStr);
    // Constructor for AC analysis (magnitude and phase)
    VoltageSource(const string& name,const string& node1,const string& node2,const string& acMagStr
        ,const string& acPhaseStr,const string& acFreqstr);
    string toString() const override;
    double getDCValue() const { return dc_value; }
    Complex getACPhasor(double analysisFrequency) const;
    double getInstantaneousValue(double time) const override;
    void stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
        const map<string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
        const vector<double>& prev_voltages,const vector<double>& prev_branch_currents) override;
};

#endif
