#include "CurrentSource.h"
#include <stdexcept> // For std::invalid_argument, std::out_of_range
#include <string>    // For std::to_string
#include <cmath>     // For std::sin, std::cos, M_PI

// Constructor for DC Current Source
CurrentSource::CurrentSource(const std::string& name,const std::string& node1,const std::string& node2,const std::string& dcValueStr)
    : Element(name,node1,node2,"CurrentSource"),
    dc_value(parseValue(dcValueStr)),
    ac_magnitude(0.0),ac_phase(0.0),ac_frequency(0.0) {
    // No specific checks for current value yet (can be negative for direction)
}

// Constructor for AC Current Source
CurrentSource::CurrentSource(const std::string& name,const std::string& node1,const std::string& node2,
    const std::string& acMagnitudeStr,const std::string& acPhaseStr,const std::string& acFrequencyStr)
    : Element(name,node1,node2,"CurrentSource"),dc_value(0.0) { // AC sources have 0 DC value initially
    this->ac_magnitude=parseValue(acMagnitudeStr);
    this->ac_phase=parseValue(acPhaseStr);
    this->ac_frequency=parseValue(acFrequencyStr);

    if(this->ac_magnitude<0) {
        throw InvalidValueError("AC magnitude for current source cannot be negative.");
    }
    if(this->ac_frequency<0) { // Frequency usually positive, 0 for DC equivalent
        throw InvalidValueError("AC frequency for current source cannot be negative.");
    }
}

// toString method implementation
std::string CurrentSource::toString() const {
    if(isACSource()) {
        return "CurrentSource "+name+" "+node1+" "+node2+" AC Mag="+std::to_string(ac_magnitude)+"A Phase="+std::to_string(ac_phase)+"deg Freq="+std::to_string(ac_frequency)+"Hz";
    }
    else {
        return "CurrentSource "+name+" "+node1+" "+node2+" DC="+std::to_string(dc_value)+"A";
    }
}

// Get the phasor for AC analysis at a given analysis frequency
Complex CurrentSource::getACPhasor(double analysisFrequency) const {
    if(isACSource()&&ac_frequency==analysisFrequency) {
        // Convert magnitude and phase to complex number
        double phase_rad=ac_phase*M_PI/180.0;
        return Complex(ac_magnitude*std::cos(phase_rad),ac_magnitude*std::sin(phase_rad));
    }
    else if(ac_frequency==0.0&&analysisFrequency==0.0) {
        // DC analysis, treat as DC value with 0 phase
        return Complex(dc_value,0.0);
    }
    else {
        // If source frequency doesn't match analysis frequency, it's a short circuit (0 current)
        // or if it's a DC source and AC analysis is being run.
        return Complex(0.0,0.0);
    }
}

// Get instantaneous value for transient analysis
double CurrentSource::getInstantaneousValue(double time) const {
    if(isACSource()) {
        // For AC source, instantaneous value is magnitude * sin(omega*t + phase)
        double omega=2*M_PI*ac_frequency;
        double phase_rad=ac_phase*M_PI/180.0;
        return ac_magnitude*std::sin(omega*time+phase_rad);
    }
    else {
        // For DC source, instantaneous value is just the DC value
        return dc_value;
    }
}

// Independent Current Sources contribute to the RHS vector, not directly to the A matrix via stamp.
// The Circuit::stampTransient method will handle their contribution to 'b'.
void CurrentSource::stampTransient(Matrix<double>& A,std::vector<double>& b,
    const std::map<std::string,int>& nodeToIndex,
    const std::map<std::string,int>& voltageSourceNameToCurrentIndex,
    double dt,double time,
    const std::vector<double>& prev_voltages,
    const std::vector<double>& prev_branch_currents) {
    // This method is intentionally empty for CurrentSource, as its contribution
    // to 'b' is handled directly in Circuit::solveTransient based on its instantaneous value.
    // The parameters are marked (void) to suppress unused variable warnings.
    (void)A; (void)b; (void)nodeToIndex; (void)voltageSourceNameToCurrentIndex; (void)dt; (void)time; (void)prev_voltages; (void)prev_branch_currents;
}