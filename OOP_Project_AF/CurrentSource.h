#pragma once
#ifndef CURRENT_SOURCE_H
#define CURRENT_SOURCE_H
#include "Element.h"
#include "Complex.h"
class CurrentSource : public Element 
{
private:
    double dc_value; // DC current value in Amperes
    // For AC analysis, current sources can have magnitude and phase
    double ac_magnitude;
    double ac_phase_deg; // Phase in degrees
    double ac_frequency; // Frequency in Hz for AC sources (for transient analysis)
public:
    // Constructor for DC analysis
    CurrentSource(const string& name,const string& node1,const string& node2,const string& valueStr);
    // Constructor for AC analysis (magnitude and phase)
    CurrentSource(const string& name,const string& node1,const string& node2,const string& acMagStr,
        const string& acPhaseStr,const string& acFreqStr);
    string toString() const override;
    double getDCValue() const { return dc_value; }
    Complex getACPhasor(double analysisFrequenc) const;
    double getInstantaneousValue(double time) const override;
    void stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
        const map<string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
        const vector<double>& prev_voltages,
        const vector<double>& prev_branch_currents) override;
};
#endif