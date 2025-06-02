#include "CurrentSource.h"
#include <string>
// Constructor for DC analysis
CurrentSource::CurrentSource(const std::string& name,const std::string& node1,const std::string& node2,const std::string& valueStr)
    : Element(name,node1,node2,"CurrentSource"),ac_magnitude(0.0),ac_phase_deg(0.0) 
{
    this->dc_value=parseValue(valueStr);
}
CurrentSource::CurrentSource(const std::string& name,const std::string& node1,const std::string& node2,
    const std::string& acMagStr,const std::string& acPhaseStr)
    : Element(name,node1,node2,"CurrentSource"),dc_value(0.0) 
{   // DC value is 0 for AC source
    this->ac_magnitude=parseValue(acMagStr);
    this->ac_phase_deg=parseValue(acPhaseStr);
}
std::string CurrentSource::toString() const 
{
    if(ac_magnitude>0)
        return "CurrentSource "+name+" "+node1+" "+node2+" AC: "+std::to_string(ac_magnitude)+"A, "+std::to_string(ac_phase_deg)+"°";
    else
        return "CurrentSource "+name+" "+node1+" "+node2+" DC: "+std::to_string(dc_value)+"A";
}
Complex CurrentSource::getACPhasor() const { return Complex::polar(ac_magnitude,ac_phase_deg*M_PI/180.0); }