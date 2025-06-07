#include "VoltageSource.h"
#include <stdexcept> // For invalid_argument, out_of_range
#include <string>    // For to_string
#include <cmath>     // For sin, cos, M_PI

using namespace std;

// Constructor for DC Voltage Source
VoltageSource::VoltageSource(const string& name,const string& node1,const string& node2,const string& dcValueStr)
    : Element(name,node1,node2,"VoltageSource"), // Changed base class constructor call
    dc_value(parseValue(dcValueStr)),
    ac_magnitude(0.0),ac_phase(0.0),ac_frequency(0.0) {
    // No specific checks for voltage value yet (can be negative for polarity)
}

// Constructor for AC Voltage Source
VoltageSource::VoltageSource(const string& name,const string& node1,const string& node2,
    const string& acMagnitudeStr,const string& acPhaseStr,const string& acFrequencyStr)
    : Element(name,node1,node2,"VoltageSource"),dc_value(0.0) { // Changed base class constructor call
    this->ac_magnitude=parseValue(acMagnitudeStr);
    this->ac_phase=parseValue(acPhaseStr);
    this->ac_frequency=parseValue(acFrequencyStr);

    if(this->ac_magnitude<0) {
        throw InvalidValueError("AC magnitude for voltage source cannot be negative.");
    }
    if(this->ac_frequency<0) { // Frequency usually positive, 0 for DC equivalent
        throw InvalidValueError("AC frequency for voltage source cannot be negative.");
    }
}

// toString method implementation
string VoltageSource::toString() const {
    if(isACSource()) {
        return "VoltageSource "+name+" "+node1+" "+node2+" AC Mag="+to_string(ac_magnitude)+"V Phase="+to_string(ac_phase)+"deg Freq="+to_string(ac_frequency)+"Hz";
    }
    else {
        return "VoltageSource "+name+" "+node1+" "+node2+" DC="+to_string(dc_value)+"V";
    }
}

// Get the phasor for AC analysis at a given analysis frequency
Complex VoltageSource::getACPhasor(double analysisFrequency) const {
    if(isACSource()&&ac_frequency==analysisFrequency) {
        // Convert magnitude and phase to complex number
        double phase_rad=ac_phase*M_PI/180.0;
        return Complex(ac_magnitude*cos(phase_rad),ac_magnitude*sin(phase_rad));
    }
    else if(ac_frequency==0.0&&analysisFrequency==0.0) {
        // DC analysis, treat as DC value with 0 phase
        return Complex(dc_value,0.0);
    }
    else {
        // If source frequency doesn't match analysis frequency, it's a short circuit (0 voltage)
        // or if it's a DC source and AC analysis is being run.
        return Complex(0.0,0.0);
    }
}

// Get instantaneous value for transient analysis
double VoltageSource::getInstantaneousValue(double time) const {
    if(isACSource()) {
        // For AC source, instantaneous value is magnitude * sin(omega*t + phase)
        double omega=2*M_PI*ac_frequency;
        double phase_rad=ac_phase*M_PI/180.0;
        return ac_magnitude*sin(omega*time+phase_rad);
    }
    else {
        // For DC source, instantaneous value is just the DC value
        return dc_value;
    }
}

// Independent Voltage Sources are handled directly by the Circuit::solveTransient method
// because they introduce a new row/column in the MNA matrix.
void VoltageSource::stampTransient(Matrix<double>& A,vector<double>& b,
    const map<string,int>& nodeToIndex,
    const map<string,int>& voltageSourceNameToCurrentIndex,
    double dt,double time,
    const vector<double>& prev_voltages,
    const vector<double>& prev_branch_currents) {
    // This method is intentionally empty for VoltageSource, as its contribution
    // to 'A' and 'b' is handled directly in Circuit::solveTransient based on its instantaneous value.
    // The parameters are marked (void) to suppress unused variable warnings.
    (void)A; (void)b; (void)nodeToIndex; (void)voltageSourceNameToCurrentIndex; (void)dt; (void)time; (void)prev_voltages; (void)prev_branch_currents;
}