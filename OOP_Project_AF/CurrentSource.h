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
public:
    // Constructor for DC analysis
    CurrentSource(const std::string& name,const std::string& node1,const std::string& node2,const std::string& valueStr);
    // Constructor for AC analysis (magnitude and phase)
    CurrentSource(const std::string& name,const std::string& node1,const std::string& node2,
        const std::string& acMagStr,const std::string& acPhaseStr);
    std::string toString() const override;
    double getDCValue() const { return dc_value; }
    Complex getACPhasor() const;
};
#endif