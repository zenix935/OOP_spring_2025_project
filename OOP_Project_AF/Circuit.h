#pragma once
#ifndef CIRCUIT_H
#define CIRCUIT_H
#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include "Element.h"
#include "Resistor.h"
#include "Capacitor.h"
#include "Inductor.h"
#include "VoltageSource.h"
#include "CurrentSource.h"
#include "Complex.h"
#include "Exceptions.h"
#include "Matrix.h"
using namespace std;
class Circuit
{
public:
	Circuit();
	vector<unique_ptr<Element>> elements; // Vector of unique pointers to elements
	set<string> nodes; // Set of unique node names
	map<string,int> nodeToIndex; // Map of node names to indices
	map<string,int> voltageSourceNameToCurrentIndex; // Maps independent V-source names to their branch current indices
	bool hasElement(const string& name) const; // Helper to check if an element name already exists (case-insensitive)
	void updateNodesAndBranchCurrentIndices(); // Helper to update nodes and independent voltage source/inductor current indices
	void buildNodeIndexMap(); // Helper to build nodeToIndex map
	// Internal state for transient analysis
	std::vector<double> prev_voltages;
	std::vector<double> prev_branch_currents;
	void addElement(const string& type,const string& name,const string& node1,const string& node2,const string valueStr); // Add an element to the circuit (for R, C, L)
	// Overload for VoltageSource/CurrentSource (DC or AC)
	// Now takes 3 value parameters for AC sources (Mag, Phase, Freq)
	void addSource(const string& type,const string& name,const string& node1,const string& node2,
		const string& val1,const string& val2="",const string& val3="");
	void deleteElement(const string& name); // Delete an element from the circuit
	void renameNode(const string& oldName,const string& newName); // Rename a node
	void listElements(const string& componentType="") const; // List all elements in the circuit
	void listNodes() const; // List all nodes in the circuit
	void buildMNAMatrix(Matrix<double>& A,vector<double>& b); // Build the Modified Nodal Analysis (MNA) matrix and right-hand side vector for linear DC
	void solveLinearDC(); // Solves the circuit for linear DC analysis using Gaussian Elimination
	void buildACMNAMatrix(Matrix<Complex>& A_ac,vector<Complex>& b_ac,double frequency); // Build the Complex MNA matrix and right-hand side vector for AC analysis
	void solveAC(double startFreq,double endFreq,int numPoints,const string& sweepType); // Performs AC analysis over a frequency range
	void solveTransient(double tstep,double tstop,double tstart,double tmaxstep); // Performs Transient analysis
	const map<string,int>& getNodeToIndexMap() const { return nodeToIndex; } // Getter for nodeToIndex (useful for displaying results)
};
#endif