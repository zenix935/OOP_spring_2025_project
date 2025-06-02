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
public:
    // Constructor for DC analysis
    VoltageSource(const std::string& name,const std::string& node1,const std::string& node2,const std::string& valueStr);
    // Constructor for AC analysis (magnitude and phase)
    VoltageSource(const std::string& name,const std::string& node1,const std::string& node2,
        const std::string& acMagStr,const std::string& acPhaseStr);
    std::string toString() const override;
    double getDCValue() const { return dc_value; }
    Complex getACPhasor() const;
};

#endif
