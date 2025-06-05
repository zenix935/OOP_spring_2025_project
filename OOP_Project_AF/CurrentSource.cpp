#include "CurrentSource.h"
#include <string>
// Constructor for DC analysis
CurrentSource::CurrentSource(const string& name,const string& node1,const string& node2,const string& valueStr)
    : Element(name,node1,node2,"CurrentSource"),ac_magnitude(0.0),ac_phase_deg(0.0),ac_frequency(0.0)
{
    this->dc_value=parseValue(valueStr);
}
// Constructor for AC analysis (magnitude, phase, frequency)
CurrentSource::CurrentSource(const string& name,const string& node1,const string& node2,const string& acMagStr,
    const string& acPhaseStr,const string& acFreqStr)
    : Element(name,node1,node2,"CurrentSource"),dc_value(0.0) 
{   // DC value is 0 for AC source
    this->ac_magnitude=parseValue(acMagStr);
    this->ac_phase_deg=parseValue(acPhaseStr);
    this->ac_frequency=parseValue(acFreqStr);
    if(this->ac_frequency<0)
        throw InvalidValueError("AC frequency cannot be negative.");
}
string CurrentSource::toString() const 
{
    if(ac_magnitude>0)
        return "CurrentSource "+name+" "+node1+" "+node2+" AC: "+to_string(ac_magnitude)+"A, "+to_string(ac_phase_deg)+
        "°, "+to_string(ac_frequency)+"Hz";
    else
        return "CurrentSource "+name+" "+node1+" "+node2+" DC: "+std::to_string(dc_value)+"A";
}
// Getter for AC phasor (complex number) - now takes analysisFrequency
Complex CurrentSource::getACPhasor(double analysisFrequency) const 
{
    // For AC analysis, the frequency is driven by the sweep.
    // The source's internal ac_frequency is primarily for transient analysis.
    // Here, we use the analysisFrequency passed in.
    return Complex::polar(ac_magnitude,ac_phase_deg*M_PI/180.0);
}
// Override to get instantaneous value for transient analysis
double CurrentSource::getInstantaneousValue(double time) const 
{
    if(ac_magnitude>0&&ac_frequency>0) 
    {
        // Sine wave: I(t) = Magnitude * sin(2 * pi * freq * t + phase_radians)
        double phase_radians=ac_phase_deg*M_PI/180.0;
        return ac_magnitude*sin(2.0*M_PI*ac_frequency*time+phase_radians);
    }
    else
        return dc_value; // For DC source or AC source with 0 magnitude/frequency
}
// Implement stampTransient for current sources
void CurrentSource::stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
    const map<string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
    const vector<double>& prev_voltages,const vector<double>& prev_branch_currents) 
{
    (void)A; // Unused in current source stamping
    (void)dt; // Unused in current source stamping
    (void)prev_voltages; // Unused in current source stamping
    (void)prev_branch_currents; // Unused in current source stamping
    (void)voltageSourceNameToCurrentIndex; // Unused in current source stamping
    double instantaneousCurrent=getInstantaneousValue(time); // Get time-varying current
    int idx1=(node1=="GND")?-1:nodeToIndex.at(node1);
    int idx2=(node2=="GND")?-1:nodeToIndex.at(node2);
    // Current from n1 to n2.
    // KCL at n1: ... - I_source = 0 => ... - instantaneousCurrent = 0
    // KCL at n2: ... + I_source = 0 => ... + instantaneousCurrent = 0
    if(idx1!=-1)
        b[idx1]-=instantaneousCurrent; // Current leaving n1 (added to RHS with negative sign)
    if(idx2!=-1)
        b[idx2]+=instantaneousCurrent; // Current entering n2 (added to RHS with positive sign)
}