#include "VoltageSource.h"
#include <string>
// Constructor for DC analysis
VoltageSource::VoltageSource(const string& name,const string& node1,const string& node2,const string& valueStr)
    : Element(name,node1,node2,"VoltageSource"),ac_magnitude(0.0),ac_phase_deg(0.0),ac_frequency(0.0)
{
    this->dc_value=parseValue(valueStr);
}
// Constructor for AC analysis (magnitude, phase, frequency)
VoltageSource::VoltageSource(const string& name,const string& node1,const string& node2,const string& acMagStr
    ,const string& acPhaseStr,const string& acFreqstr)
    : Element(name,node1,node2,"VoltageSource"),dc_value(0.0) 
{   // DC value is 0 for AC source
    this->ac_magnitude=parseValue(acMagStr);
    this->ac_phase_deg=parseValue(acPhaseStr);
	this->ac_frequency=parseValue(acFreqstr);
	if(this->ac_frequency<0.0)
		throw InvalidValueError("AC frequency cannot be negative.");
}
string VoltageSource::toString() const 
{
    if(ac_magnitude>0)
        return "VoltageSource "+name+" "+node1+" "+node2+" AC: "+to_string(ac_magnitude)+"V, "
        +to_string(ac_phase_deg)+"°, "+to_string(ac_frequency)+"Hz";
    else
        return "VoltageSource "+name+" "+node1+" "+node2+" DC: "+to_string(dc_value)+"V";
}
// Getter for AC phasor (complex number) - now takes analysisFrequency
Complex VoltageSource::getACPhasor(double analysisFrequency) const 
{
    // For AC analysis, the frequency is driven by the sweep.
    // The source's internal ac_frequency is primarily for transient analysis.
    // Here, we use the analysisFrequency passed in.
    return Complex::polar(ac_magnitude,ac_phase_deg*M_PI/180.0);
}
// Override to get instantaneous value for transient analysis
double VoltageSource::getInstantaneousValue(double time) const 
{
    if(ac_magnitude>0&&ac_frequency>0) 
    {
        // Sine wave: V(t) = Magnitude * sin(2 * pi * freq * t + phase_radians)
        double phase_radians=ac_phase_deg*M_PI/180.0;
        return ac_magnitude*sin(2.0*M_PI*ac_frequency*time+phase_radians);
    }
    else
        return dc_value; // For DC source or AC source with 0 magnitude/frequency
}
// Implement stampTransient for voltage sources
void VoltageSource::stampTransient(Matrix<double>& A,vector<double>& b,const map<string,int>& nodeToIndex,
    const map<string,int>& voltageSourceNameToCurrentIndex,double dt,double time,
    const vector<double>& prev_voltages,const vector<double>& prev_branch_currents) 
{
    (void)dt; // Unused in voltage source stamping
    (void)prev_voltages; // Unused in voltage source stamping
    (void)prev_branch_currents; // Unused in voltage source stamping
    int idx1=(node1=="GND")?-1:nodeToIndex.at(node1);
    int idx2=(node2=="GND")?-1:nodeToIndex.at(node2);
    int branchIdx=voltageSourceNameToCurrentIndex.at(name); // Get branch current index
    // KCL equation contributions (rows corresponding to nodes)
    if(idx1!=-1)
        A.add(idx1,branchIdx,1.0); // +1 coefficient for branch current leaving n1
    if(idx2!=-1)
        A.add(idx2,branchIdx,-1.0); // -1 coefficient for branch current entering n2
    // Voltage constraint equation (row corresponding to branch current)
    // V_n1 - V_n2 = Instantaneous_Voltage
    if(idx1!=-1)
        A.add(branchIdx,idx1,1.0);
    if(idx2!=-1)
        A.add(branchIdx,idx2,-1.0);
    b[branchIdx]+=getInstantaneousValue(time); // RHS for voltage constraint (time-varying)
}