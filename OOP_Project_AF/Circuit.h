#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <vector>
#include <string>
#include <set>
#include <map> // For node mapping
#include <memory> // For unique_ptr
#include <variant> // For variant (to store last analysis type)

// Include element headers
#include "Element.h" // Changed from CircuitElement.h
#include "Resistor.h"
#include "Capacitor.h"
#include "Inductor.h"
#include "VoltageSource.h"
#include "CurrentSource.h"

#include "Exceptions.h" // Include custom exceptions
#include "Matrix.h"     // Include the Matrix class (now templated)
#include "Complex.h"    // Include Complex number class

using namespace std;

class Circuit {
private:
    vector<unique_ptr<Element>> elements; // Changed type to Element
    set<string> nodes; // Using set for unique and sorted nodes
    map<string,int> nodeToIndex; // Maps node names to their matrix indices
    map<string,int> voltageSourceNameToCurrentIndex; // Maps independent V-source and Inductor names to their branch current indices

    // Helper to check if an element name already exists (case-insensitive)
    bool hasElement(const string& name) const;

    // Helper to update nodes and independent voltage source/inductor current indices
    void updateNodesAndBranchCurrentIndices();

    // Helper to build nodeToIndex map
    void buildNodeIndexMap();

    // Store results of the last performed analysis for .print command
    string last_analysis_type; // "NONE", "DC", "AC", "TRANSIENT"
    vector<double> last_dc_solution; // Stores node voltages and branch currents for DC
    map<string,Complex> last_ac_solution_phasors; // Stores node voltages and branch currents for AC (phasors)
    double last_ac_frequency; // Stores the frequency for the last AC solution

    // Transient analysis results storage
    // prev_voltages and prev_branch_currents are used as the *last* solved step for .print in transient.
    vector<double> prev_voltages;
    vector<double> prev_branch_currents;


public:
    Circuit(); // Constructor

    // Add an element to the circuit (for R, C, L)
    void addElement(const string& type,const string& name,const string& node1,const string& node2,const string& valueStr);

    // Overload for VoltageSource/CurrentSource (DC or AC)
    // Now takes 3 value parameters for AC sources (Mag, Phase, Freq)
    void addSource(const string& type,const string& name,const string& node1,const string& node2,
        const string& val1,const string& val2="",const string& val3="");

    // Delete an element from the circuit
    void deleteElement(const string& name);

    // Rename a node
    void renameNode(const string& oldName,const string& newName);

    // List all elements in the circuit
    void listElements(const string& componentType="") const;

    // List all nodes in the circuit
    void listNodes() const;

    // Build the Modified Nodal Analysis (MNA) matrix and right-hand side vector for linear DC
    void buildMNAMatrix(Matrix<double>& A,vector<double>& b);

    // Solves the circuit for linear DC analysis using Gaussian Elimination
    void solveLinearDC();

    // Performs DC sweep for a source
    void solveDCSweep(const string& sourceName,double startVal,double endVal,int numPoints); // NEW

    // Build the Complex MNA matrix and right-hand side vector for AC analysis
    void buildACMNAMatrix(Matrix<Complex>& A_ac,vector<Complex>& b_ac,double frequency);

    // Performs AC analysis over a frequency range
    void solveAC(double startFreq,double endFreq,int numPoints,const string& sweepType);

    // Performs Transient analysis
    void solveTransient(double tstep,double tstop,double tstart,double tmaxstep);

    // Prints specific results (voltages or currents) from the last analysis
    void printResults(const vector<string>& whatToPrint) const; // NEW

    // Getter for nodeToIndex (useful for displaying results)
    const map<string,int>& getNodeToIndexMap() const { return nodeToIndex; }
};

#endif // CIRCUIT_H