#include "Circuit.h"
#include <iostream>
#include <algorithm> 
#include <map>       
#include <cmath>     
#include <iomanip>   
#include <regex>     
#include <fstream>   
using namespace std;
// Helper to check if an element name already exists (case-insensitive)
bool Circuit::hasElement(const string& name) const 
{
    string upperName=toUpper(name);
    for(const auto& el:elements)
        if(toUpper(el->getName())==upperName)
            return true;
    return false;
}

// Helper to update nodes and independent voltage source/inductor current indices
void Circuit::updateNodesAndBranchCurrentIndices() 
{
    nodes.clear();
    nodes.insert("GND"); // Always include GND
    voltageSourceNameToCurrentIndex.clear(); // Clear previous indices
    for(const auto& el:elements) 
    {
        nodes.insert(el->getNode1());
        nodes.insert(el->getNode2());
    }
    buildNodeIndexMap(); // Rebuild node index map
    // Assign unique indices for independent voltage sources AND inductors
    int currentBranchIndex=nodes.size()-1; // Start after node voltage indices
    for(const auto& el:elements) 
    {
        string type=toUpper(el->getType());
        if(type=="VOLTAGESOURCE"||type=="INDUCTOR") // Both add branch currents
            voltageSourceNameToCurrentIndex[el->getName()]=currentBranchIndex++;
    }
}

// Helper to build nodeToIndex map
void Circuit::buildNodeIndexMap() 
{
    nodeToIndex.clear();
    int index=0;
    for(const auto& node:nodes) 
    {
        if(node=="GND")
            nodeToIndex[node]=-1; // Ground node often has index -1 or 0 and is handled separately
        else
            nodeToIndex[node]=index++;
    }
}
Circuit::~Circuit()=default;
Circuit::Circuit() : last_analysis_type("NONE"),last_ac_frequency(0.0) 
{
    nodes.insert("GND"); // Initialize with a default ground node
    updateNodesAndBranchCurrentIndices(); // Initial map build
}
// Add an element to the circuit (for R, C, L)
void Circuit::addElement(const string& type,const string& name,const string& node1,const string& node2,const string& valueStr) 
{
    if(hasElement(name))
        throw DuplicateNameError(name,type);
    unique_ptr<Element> newElement;
    string upperType=toUpper(type);
    if(upperType=="R"||upperType=="RESISTOR")
        newElement=make_unique<Resistor>(name,node1,node2,valueStr);
    else if(upperType=="C"||upperType=="CAPACITOR")
        newElement=make_unique<Capacitor>(name,node1,node2,valueStr);
    else if(upperType=="L"||upperType=="INDUCTOR")
        newElement=make_unique<Inductor>(name,node1,node2,valueStr);
    else
        throw UnsupportedTypeError("Unsupported element type for this 'add' overload: "+type+". Use 'addSource' for V/I sources.");
    elements.push_back(move(newElement));
    updateNodesAndBranchCurrentIndices(); // Update nodes and rebuild map after adding
    cout<<"SUCCESS: Added "<<elements.back()->toString()<<endl;
}

void Circuit::addSource(const string& type,const string& name,const string& node1,const string& node2,
    const string& val1,const string& val2,const string& val3) 
{
    if(hasElement(name))
        throw DuplicateNameError(name,type);
    unique_ptr<Element> newElement;
    string upperType=toUpper(type);
    if(upperType=="V"||upperType=="VOLTAGESOURCE") 
    {
        if(val2.empty()) // DC Voltage Source (only val1 provided)
            newElement=make_unique<VoltageSource>(name,node1,node2,val1);
        else if(!val3.empty()) // AC Voltage Source (Mag, Phase, Freq)
            newElement=make_unique<VoltageSource>(name,node1,node2,val1,val2,val3);
        else
            throw SyntaxError("Invalid 'addsource V' command format. Expected: addsource V <name> <n1> <n2> <DC_value> OR addsource V <name> <n1> <n2> AC <AC_mag> <AC_phase> <AC_freq>");
    }
    else if(upperType=="I"||upperType=="CURRENTSOURCE") 
    {
        if(val2.empty()) // DC Current Source (only val1 provided)
            newElement=make_unique<CurrentSource>(name,node1,node2,val1);
        else if(!val3.empty()) // AC Current Source (Mag, Phase, Freq)
            newElement=make_unique<CurrentSource>(name,node1,node2,val1,val2,val3);
        else
            throw SyntaxError("Invalid 'addsource I' command format. Expected: addsource I <name> <n1> <n2> <DC_value> OR addsource I <name> <n1> <n2> AC <AC_mag> <AC_phase> <AC_freq>");
    }
    else
        throw UnsupportedTypeError("Unsupported source type: "+type+". Use 'add' for R, C, L.");
    elements.push_back(move(newElement));
    updateNodesAndBranchCurrentIndices();
    cout<<"SUCCESS: Added "<<elements.back()->toString()<<endl;
}

// Delete an element from the circuit
void Circuit::deleteElement(const string& name) 
{
    string upperName=toUpper(name);
    auto it=remove_if(elements.begin(),elements.end(),[&upperName](const unique_ptr<Element>& el) 
        {
            return toUpper(el->getName())==upperName;
        });
    if(it==elements.end()) // Element not found
        throw ElementNotFoundError(name);
    elements.erase(it,elements.end());
    updateNodesAndBranchCurrentIndices(); // Update nodes and rebuild map after deleting
    cout<<"SUCCESS: Deleted element "<<upperName<<endl;
}
// Rename a node
void Circuit::renameNode(const string& oldName,const string& newName) 
{
    string upperOldName=toUpper(oldName);
    string upperNewName=toUpper(newName);
    if(nodes.find(upperOldName)==nodes.end())
        throw NodeNotFoundError(oldName);
    if(nodes.count(upperNewName)&&upperOldName!=upperNewName)
        throw DuplicateNameError(newName,"Node name");
    // Update nodes in elements
    for(auto& el:elements) 
    {
        if(toUpper(el->getNode1())==upperOldName)
            el->setNode1(upperNewName);
        if(toUpper(el->getNode2())==upperOldName)
            el->setNode2(upperNewName);
    }
    updateNodesAndBranchCurrentIndices(); // Rebuild the nodes set and map based on updated elements
    cout<<"SUCCESS: Node renamed from "<<upperOldName<<" to "<<upperNewName<<endl;
}

// List all elements in the circuit
void Circuit::listElements(const string& componentType) const 
{
    if(elements.empty()) 
    {
        cout<<"No elements added yet."<<endl;
        return;
    }
    string upperComponentType=toUpper(componentType);
    bool foundAny=false;
    if(upperComponentType.empty()) 
    {
        cout<<"All Circuit Elements:"<<endl;
        for(const auto& el:elements) 
        {
            cout<<"- "<<el->toString()<<endl;
            foundAny=true;
        }
    }
    else 
    {
        cout<<upperComponentType<<" Elements:"<<endl;
        for(const auto& el:elements) 
        {
            if(toUpper(el->getType()).find(upperComponentType)!=string::npos) 
            {
                cout<<"- "<<el->toString()<<endl;
                foundAny=true;
            }
        }
        if(!foundAny)
            cout<<"No "<<upperComponentType<<" elements found."<<endl;
    }
}

// List all nodes in the circuit
void Circuit::listNodes() const 
{
    cout<<"Available nodes: ";
    bool first=true;
    for(const auto& node:nodes) 
    {
        if(!first)
            cout<<", ";
        cout<<node;
        first=false;
    }
    cout<<endl;
}

// Build the Modified Nodal Analysis (MNA) matrix and right-hand side vector for linear DC
void Circuit::buildMNAMatrix(Matrix<double>& A,vector<double>& b) 
{
    // Ground node is not included in the matrix size for node voltages
    int numNonGNDNodes=nodes.size()-1;
    // Total MNA matrix size = (number of non-GND nodes) + (number of independent voltage sources and inductors that add branch currents)
    int mnaSize=numNonGNDNodes+voltageSourceNameToCurrentIndex.size();
    // Resize matrix A and vector b
    A.resize(mnaSize,mnaSize);
    b.assign(mnaSize,0.0); // Initialize b with zeros
    for(const auto& element:elements) 
    {
        string type=toUpper(element->getType());
        string n1=toUpper(element->getNode1());
        string n2=toUpper(element->getNode2());
        int idx1=(n1=="GND")?-1:nodeToIndex.at(n1);
        int idx2=(n2=="GND")?-1:nodeToIndex.at(n2);
        // Handle Resistors
        if(type=="RESISTOR")
        {
            const Resistor* r=static_cast<const Resistor*>(element.get());
            double conductance=1.0/r->getValue();
            if(idx1!=-1)
                A.add(idx1,idx1,conductance);
            if(idx2!=-1)
                A.add(idx2,idx2,conductance);
            if(idx1!=-1&&idx2!=-1)
            {
                A.add(idx1,idx2,-conductance);
                A.add(idx2,idx1,-conductance);
            }
        }
        // Handle Independent Current Sources
        else if(type=="CURRENTSOURCE") 
        {
            const CurrentSource* is=static_cast<const CurrentSource*>(element.get());
            double current=is->getDCValue(); // Use DC value
            if(idx1!=-1)
                b[idx1]-=current; // Current leaving n1 (added to RHS with negative sign)
            if(idx2!=-1)
                b[idx2]+=current; // Current entering n2 (added to RHS with positive sign)
        }
        // Handle Independent Voltage Sources
        else if(type=="VOLTAGESOURCE") 
        {
            const VoltageSource* vs=static_cast<const VoltageSource*>(element.get());
            double voltage=vs->getDCValue(); // Use DC value
            int branchIdx=voltageSourceNameToCurrentIndex.at(vs->getName());
            // KCL equation contributions (rows corresponding to nodes)
            if(idx1!=-1)
                A.add(idx1,branchIdx,1.0); // +1 coefficient for branch current leaving n1
            if(idx2!=-1)
                A.add(idx2,branchIdx,-1.0); // -1 coefficient for branch current entering n2
            // Voltage constraint equation (row corresponding to branch current)
            // V_n1 - V_n2 = Voltage
            if(idx1!=-1)
                A.add(branchIdx,idx1,1.0);
            if(idx2!=-1)
                A.add(branchIdx,idx2,-1.0);
            b[branchIdx]+=voltage; // RHS for voltage constraint
        }
        // Handle Capacitors (for DC analysis, C is open circuit)
        else if(type=="CAPACITOR")
        {
            // For DC analysis, capacitors are open circuits, so they don't contribute to MNA.
        }
        // Handle Inductors (for DC analysis, L is short circuit)
        else if(type=="INDUCTOR") 
        {
            // For DC analysis, inductors are short circuits. This means V_n1 - V_n2 = 0.
            // This requires adding a branch current variable for the inductor.
            const Inductor* l=static_cast<const Inductor*>(element.get());
            int branchIdx=voltageSourceNameToCurrentIndex.at(l->getName()); // Get branch current index
            // KCL equation contributions (rows corresponding to nodes)
            if(idx1!=-1)
                A.add(idx1,branchIdx,1.0); // +1 coefficient for branch current leaving n1
            if(idx2!=-1)
                A.add(idx2,branchIdx,-1.0); // -1 coefficient for branch current entering n2
            // Voltage constraint equation (row corresponding to branch current)
            // V_n1 - V_n2 = 0 (short circuit)
            if(idx1!=-1)
                A.add(branchIdx,idx1,1.0);
            if(idx2!=-1)
                A.add(branchIdx,idx2,-1.0);
            // b[branchIdx] remains 0 for this constraint
        }
    }
}

// Solves the circuit for linear DC analysis using Gaussian Elimination
void Circuit::solveLinearDC() 
{
    if(elements.empty()) 
    {
        cout<<"Circuit is empty. Nothing to analyze."<<endl;
        last_analysis_type="NONE";
        return;
    }
    Matrix<double> A(0,0); 
    vector<double> b;
    try 
    {
        // Build the linear MNA matrix (A) and RHS vector (b)
        buildMNAMatrix(A,b);
        // Solve the linear system A * x = b using Gaussian Elimination
        vector<double> solutionVector=A.solveGaussianElimination(b);
        last_dc_solution=solutionVector; // Store the solution
        last_analysis_type="DC"; // Set analysis type
        cout<<"\n--- DC Analysis Results (Linear Circuit) ---\n";
        // Print node voltages
        for(const auto& pair:nodeToIndex) 
        {
            if(pair.second!=-1) // Not GND
                cout<<"Node "<<pair.first<<": "<<fixed<<setprecision(6)<<solutionVector[pair.second]<<"V\n";
            else
                cout<<"Node GND: 0.000000V\n";
        }
        // Print branch currents for independent voltage sources and inductors
        for(const auto& pair:voltageSourceNameToCurrentIndex)
            cout<<"Current through "<<pair.first<<": "<<fixed<<setprecision(6)<<solutionVector[pair.second]<<"A\n";
        cout<<"--------------------------------------\n";
    }
    catch(const CircuitError& e) 
    {
        cerr<<"DC Analysis Error: "<<e.what()<<endl;
        last_analysis_type="NONE"; // Indicate no valid solution
    }
    catch(const exception& e) 
    {
        cerr<<"An unexpected error occurred during DC analysis: "<<e.what()<<endl;
        last_analysis_type="NONE"; // Indicate no valid solution
    }
}

// Performs DC sweep for a source
void Circuit::solveDCSweep(const string& sourceName,double startVal,double endVal,int numPoints) 
{
    if(elements.empty()) 
    {
        cout<<"Circuit is empty. Nothing to analyze."<<endl;
        last_analysis_type="NONE";
        return;
    }
    if(numPoints<=1&&startVal!=endVal)  // If only one point, start and end must be same for meaningful sweep
        throw CircuitError("Number of points for DC sweep must be greater than 1, or start and end values must be identical for a single point.");
    cout<<"\n--- Starting DC Sweep Analysis ---\n";
    cout<<"Sweeping source '"<<sourceName<<"' from "<<startVal<<" to "<<endVal<<" with "<<numPoints<<" points.\n";
    // Find the source by name
    Element* targetSource=nullptr;
    for(auto& el:elements) 
    {
        if(toUpper(el->getName())==toUpper(sourceName)) 
        {
            targetSource=el.get();
            break;
        }
    }
    if(!targetSource)
        throw ElementNotFoundError("Source with name '"+sourceName+"' not found.");
    // Store original DC value
    double originalDCValue=0.0;
    string sourceType=toUpper(targetSource->getType());
    if(sourceType=="VOLTAGESOURCE")
        originalDCValue=static_cast<VoltageSource*>(targetSource)->getDCValue();
    else if(sourceType=="CURRENTSOURCE")
        originalDCValue=static_cast<CurrentSource*>(targetSource)->getDCValue();
    else
        throw UnsupportedTypeError("DC sweep only supported for VoltageSource or CurrentSource elements.");
    // Prepare results header
    cout<<setw(15)<<"Source_Value";
    for(const auto& pair:nodeToIndex)
        if(pair.second!=-1)
            cout<<setw(15)<<"V("+pair.first+")";
    // Include all branch currents (voltage sources and inductors)
    for(const auto& el:elements) 
    {
        string type=toUpper(el->getType());
        if(type=="VOLTAGESOURCE"||type=="INDUCTOR")
            cout<<setw(15)<<"I("+el->getName()+")";
    }
    cout<<"\n";
    try 
    {
        for(int i=0; i<numPoints; ++i) 
        {
            double currentValue;
            if(numPoints==1)
                currentValue=startVal;
            else
                currentValue=startVal+(endVal-startVal)*i/(numPoints-1);
            // Temporarily set the source's DC value
            if(sourceType=="VOLTAGESOURCE")
                static_cast<VoltageSource*>(targetSource)->setDCValue(currentValue);
            else if(sourceType=="CURRENTSOURCE")
                static_cast<CurrentSource*>(targetSource)->setDCValue(currentValue);
            Matrix<double> A(0,0);
            vector<double> b;
            buildMNAMatrix(A,b);
            vector<double> solutionVector=A.solveGaussianElimination(b);
            // Store the last sweep point solution (last_dc_solution will hold the last iteration)
            last_dc_solution=solutionVector;
            last_analysis_type="DC"; // Mark as DC even if it's a sweep
            // Print results for current sweep value
            cout<<fixed<<setprecision(4)<<setw(15)<<currentValue;
            for(const auto& pair:nodeToIndex)
                if(pair.second!=-1)
                    cout<<setw(15)<<fixed<<setprecision(4)<<solutionVector[pair.second];
            for(const auto& el:elements) 
            {
                string type=toUpper(el->getType());
                if(type=="VOLTAGESOURCE"||type=="INDUCTOR") 
                {
                    auto it=voltageSourceNameToCurrentIndex.find(el->getName());
                    if(it!=voltageSourceNameToCurrentIndex.end())
                        cout<<setw(15)<<fixed<<setprecision(4)<<solutionVector[it->second];
                    else
                        cout<<setw(15)<<"N/A";
                }
            }
            cout<<"\n";
        }
    }
    catch(const CircuitError& e) 
    {
        cerr<<"DC Sweep Error: "<<e.what()<<endl;
        last_analysis_type="NONE";
    }
    catch(const exception& e) 
    {
        cerr<<"An unexpected error occurred during DC sweep: "<<e.what()<<endl;
        last_analysis_type="NONE";
    }
    // Restore original DC value of the source
    if(sourceType=="VOLTAGESOURCE")
        static_cast<VoltageSource*>(targetSource)->setDCValue(originalDCValue);
    else if(sourceType=="CURRENTSOURCE")
        static_cast<CurrentSource*>(targetSource)->setDCValue(originalDCValue);
    cout<<"--- DC Sweep Analysis Complete ---\n";
}

// Build the Complex MNA matrix and right-hand side vector for AC analysis
void Circuit::buildACMNAMatrix(Matrix<Complex>& A_ac,vector<Complex>& b_ac,double frequency) 
{
    // Ground node is not included in the matrix size for node voltages
    int numNonGNDNodes=nodes.size()-1;
    // Total MNA matrix size = (number of non-GND nodes) + (number of independent voltage sources and inductors that add branch currents)
    int mnaSize=numNonGNDNodes+voltageSourceNameToCurrentIndex.size();
    // Resize matrix A and vector b
    A_ac.resize(mnaSize,mnaSize);
    b_ac.assign(mnaSize,Complex(0.0)); // Initialize b_ac with complex zeros
    for(const auto& element:elements) 
    {
        string type=toUpper(element->getType());
        string n1=toUpper(element->getNode1());
        string n2=toUpper(element->getNode2());
        int idx1=(n1=="GND")?-1:nodeToIndex.at(n1);
        int idx2=(n2=="GND")?-1:nodeToIndex.at(n2);
        // Get complex admittance for R, C, L
        Complex admittance=element->getComplexAdmittance(frequency);
        // Handle R, C, L contributions to the MNA matrix
        if(type=="RESISTOR"||type=="CAPACITOR") 
        { 
            if(idx1!=-1)
                A_ac.add(idx1,idx1,admittance);
            if(idx2!=-1)
                A_ac.add(idx2,idx2,admittance);
            if(idx1!=-1&&idx2!=-1)
                A_ac.add(idx1,idx2,-admittance);
                A_ac.add(idx2,idx1,-admittance);
        }
        // Handle Inductors (as they add a branch current)
        else if(type=="INDUCTOR") 
        {
            const Inductor* l=static_cast<const Inductor*>(element.get());
            int branchIdx=voltageSourceNameToCurrentIndex.at(l->getName()); // Get branch current index
            // KCL equation contributions (rows corresponding to nodes)
            if(idx1!=-1)
                A_ac.add(idx1,branchIdx,Complex(1.0)); // +1 coefficient for branch current leaving n1
            if(idx2!=-1)
                A_ac.add(idx2,branchIdx,Complex(-1.0)); // -1 coefficient for branch current entering n2
            // Voltage constraint equation (row corresponding to branch current)
            // V_n1 - V_n2 - Z_L * I_L = 0 => V_n1 - V_n2 - (1/Y_L) * I_L = 0
            // Or, V_n1 - V_n2 = Z_L * I_L
            // Z_L = j * omega * L
            Complex impedance=Complex(1.0)/admittance; // Impedance of inducto
            if(idx1!=-1)
                A_ac.add(branchIdx,idx1,Complex(1.0));
            if(idx2!=-1)
                A_ac.add(branchIdx,idx2,Complex(-1.0));
            A_ac.add(branchIdx,branchIdx,-impedance); // Coefficient for inductor current variable
            // b_ac[branchIdx] remains 0 for this constraint
        }
        // Handle Independent Current Sources
        else if(type=="CURRENTSOURCE") 
        {
            const CurrentSource* is=static_cast<const CurrentSource*>(element.get());
            // For AC analysis, use the analysis frequency to determine the source's phasor
            Complex currentPhasor=is->getACPhasor(frequency);
            if(idx1!=-1)
                b_ac[idx1]-=currentPhasor; // Current leaving n1
            if(idx2!=-1)
                b_ac[idx2]+=currentPhasor; // Current entering n2
        }
        // Handle Independent Voltage Sources
        else if(type=="VOLTAGESOURCE") 
        {
            const VoltageSource* vs=static_cast<const VoltageSource*>(element.get());
            // For AC analysis, use the analysis frequency to determine the source's phasor
            Complex voltagePhasor=vs->getACPhasor(frequency);
            int branchIdx=voltageSourceNameToCurrentIndex.at(vs->getName());
            // KCL equation contributions (rows corresponding to nodes)
            if(idx1!=-1)
                A_ac.add(idx1,branchIdx,Complex(1.0)); // +1 coefficient for branch current leaving n1
            if(idx2!=-1)
                A_ac.add(idx2,branchIdx,Complex(-1.0)); // -1 coefficient for branch current entering n2
            // Voltage constraint equation (row corresponding to branch current)
            // V_n1 - V_n2 = VoltagePhasor
            if(idx1!=-1)
                A_ac.add(branchIdx,idx1,Complex(1.0));
            if(idx2!=-1)
                A_ac.add(branchIdx,idx2,Complex(-1.0));
            b_ac[branchIdx]+=voltagePhasor; // RHS for voltage constraint
        }
    }
}

// Performs AC analysis over a frequency range
void Circuit::solveAC(double startFreq,double endFreq,int numPoints,const string& sweepType) 
{
    if(elements.empty()) 
    {
        cout<<"Circuit is empty. Nothing to analyze."<<endl;
        last_analysis_type="NONE";
        return;
    }
    if(numPoints<=0)
        throw CircuitError("Number of points for AC sweep must be positive.");
    if(startFreq<=0||endFreq<=0)
        throw CircuitError("Start and end frequencies must be positive for AC analysis.");
    if(startFreq>endFreq)
        throw CircuitError("Start frequency cannot be greater than end frequency.");
    cout<<"\n--- Starting AC Analysis ---\n";
    cout<<"Frequency range: "<<startFreq<<" Hz to "<<endFreq<<" Hz\n";
    cout<<"Number of points: "<<numPoints<<", Sweep type: "<<sweepType<<"\n";
    // Prepare results header
    cout<<setw(15)<<"Frequency (Hz)";
    for(const auto& pair:nodeToIndex) 
        if(pair.second!=-1)
            cout<<setw(15)<<"V("+pair.first+") Mag"<<setw(15)<<"V("+pair.first+") Phase";
    // Iterate through elements to find voltage sources and inductors for current printing
    for(const auto& el:elements) 
    {
        string type=toUpper(el->getType());
        if(type=="VOLTAGESOURCE"||type=="INDUCTOR")
            cout<<setw(15)<<"I("+el->getName()+") Mag"<<setw(15)<<"I("+el->getName()+") Phase";
    }
    cout<<"\n";
    try 
    {
        for(int i=0; i<numPoints; ++i) 
        {
            double currentFreq;
            if(sweepType=="LIN")
                currentFreq=startFreq+(endFreq-startFreq)*i/(numPoints-1);
            else if(sweepType=="DEC"||sweepType=="OCT") 
            {
                // For logarithmic sweep, calculate log-spaced frequencies
                double logStart=log10(startFreq);
                double logEnd=log10(endFreq);
                currentFreq=pow(10,logStart+(logEnd-logStart)*i/(numPoints-1));
            }
            else
                throw CircuitError("Unsupported sweep type: "+sweepType+". Use LIN, DEC, or OCT.");
            if(currentFreq<=0) currentFreq=1e-9; // Avoid zero or negative frequency issues for log/division
            Matrix<Complex> A_ac(0,0);
            vector<Complex> b_ac;
            buildACMNAMatrix(A_ac,b_ac,currentFreq);
            vector<Complex> solutionVector_ac=A_ac.solveGaussianElimination(b_ac);
            // Store the last AC solution (last iteration of the sweep)
            last_ac_solution_phasors.clear(); // Clear for each point in sweep, only last point kept
            last_analysis_type="AC";
            last_ac_frequency=currentFreq;
            // Print results for current frequency
            cout<<fixed<<setprecision(4)<<setw(15)<<currentFreq;
            for(const auto& pair:nodeToIndex) 
            {
                if(pair.second!=-1) 
                {
                    Complex nodeVoltage=solutionVector_ac[pair.second];
                    cout<<setw(15)<<nodeVoltage.magnitude()<<setw(15)<<nodeVoltage.angleDegrees();
                    last_ac_solution_phasors["V("+pair.first+")"]=nodeVoltage; // Store for .print
                }
            }
            for(const auto& el:elements) 
            {
                string type=toUpper(el->getType());
                if(type=="VOLTAGESOURCE"||type=="INDUCTOR") 
                {
                    auto it=voltageSourceNameToCurrentIndex.find(el->getName());
                    if(it!=voltageSourceNameToCurrentIndex.end()) 
                    {
                        Complex branchCurrent=solutionVector_ac[it->second];
                        cout<<setw(15)<<branchCurrent.magnitude()<<setw(15)<<branchCurrent.angleDegrees();
                        last_ac_solution_phasors["I("+el->getName()+")"]=branchCurrent; // Store for .print
                    }
                    else // This case should ideally not happen if updateNodesAndBranchCurrentIndices is correct
                        cout<<setw(15)<<"N/A"<<setw(15)<<"N/A";
                }
            }
            cout<<"\n";
        }
    }
    catch(const CircuitError& e) 
    {
        cerr<<"Error during AC Analysis: "<<e.what()<<"\n";
        last_analysis_type="NONE";
        // Print empty results or error indication for this frequency point
        cout<<fixed<<setprecision(4)<<setw(15)<<"ERROR";
        // Determine the total number of output columns for error printing
        size_t numOutputColumns=(nodes.size()-1)*2; // Node voltages (Mag/Phase)
        for(const auto& el:elements) 
        {
            string type=toUpper(el->getType());
            if(type=="VOLTAGESOURCE"||type=="INDUCTOR")
                numOutputColumns+=2; // Branch currents (Mag/Phase)
        }
        for(size_t k=0; k<numOutputColumns; ++k)
            cout<<setw(15)<<"ERROR";
        cout<<"\n";
    }
    cout<<"--- AC Analysis Complete ---\n";
}

// Performs Transient analysis
void Circuit::solveTransient(double tstep,double tstop,double tstart,double tmaxstep) 
{
    if(elements.empty()) 
    {
        cout<<"Circuit is empty. Nothing to analyze."<<endl;
        last_analysis_type="NONE";
        return;
    }
    if(tstep<=0||tstop<=0)
        throw CircuitError("Tstep and Tstop must be positive for transient analysis.");
    if(tstart < 0||tstart > tstop)
        throw CircuitError("Tstart must be non-negative and less than or equal to Tstop.");
    if(tmaxstep<=0)
        tmaxstep=tstep; // Default tmaxstep to tstep if not provided or invalid
    if(tmaxstep<tstep) 
    {
        cerr<<"Warning: Tmaxstep ("<<tmaxstep<<") is less than Tstep ("<<tstep<<"). Setting Tmaxstep to Tstep.\n";
        tmaxstep=tstep;
    }
    cout<<"\n--- Starting Transient Analysis ---\n";
    cout<<"Tstep: "<<tstep<<"s, Tstop: "<<tstop<<"s, Tstart: "<<tstart<<"s, Tmaxstep: "<<tmaxstep<<"s\n";
    // Initialize previous states of reactive elements
    for(auto& el:elements) 
        el->initializeTransientState();
    // Determine the size of the MNA matrix for transient analysis
    int numNonGNDNodes=nodes.size()-1;
    // Find the maximum branch index to correctly size prev_branch_currents vector
    // This is crucial because voltageSourceNameToCurrentIndex maps directly to MNA indices
    int maxBranchIdxAssigned=-1;
    for(const auto& pair:voltageSourceNameToCurrentIndex)
        if(pair.second>maxBranchIdxAssigned)
            maxBranchIdxAssigned=pair.second;
    // mnaSize is the total number of variables (node voltages + branch currents)
    int mnaSize=numNonGNDNodes+voltageSourceNameToCurrentIndex.size();
    // Initialize prev_voltages and prev_branch_currents (class members)
    // These hold the solution from the *previous* time step for the *next* stamping.
    // They are also the "last solved" values for the .print command.
    prev_voltages.assign(numNonGNDNodes,0.0); // Node voltages (indices 0 to numNonGNDNodes-1 in solution)
    // prev_branch_currents should be sized to cover all assigned branch indices in the MNA matrix.
    // If no branch currents are present, maxBranchIdxAssigned will be -1, size will be 0.
    prev_branch_currents.assign(maxBranchIdxAssigned+1,0.0);
    last_analysis_type="TRANSIENT"; // Set analysis type
    // Prepare results header
    cout<<setw(15)<<"Time (s)";
    for(const auto& pair:nodeToIndex)
        if(pair.second!=-1)
            cout<<setw(15)<<"V("+pair.first+")";
    for(const auto& el:elements) 
    {
        string type=toUpper(el->getType());
        if(type=="VOLTAGESOURCE"||type=="INDUCTOR")
            cout<<setw(15)<<"I("+el->getName()+")";
    }
    cout<<"\n";
    double currentTime=0.0;
    double actual_dt=tstep; // For fixed time step, actual_dt is always tstep
    while(currentTime<=tstop+1e-9) 
    { // Add a small epsilon to ensure tstop is included
        Matrix<double> A(0,0);
        vector<double> b;
        A.resize(mnaSize,mnaSize);
        b.assign(mnaSize,0.0);
        for(auto& el:elements) 
        {
            string type=toUpper(el->getType());
            if(type=="RESISTOR") 
            {
                const Resistor* r=static_cast<const Resistor*>(el.get());
                double conductance=1.0/r->getValue();
                int idx1=(r->getNode1()=="GND")?-1:nodeToIndex.at(r->getNode1());
                int idx2=(r->getNode2()=="GND")?-1:nodeToIndex.at(r->getNode2());
                if(idx1!=-1) A.add(idx1,idx1,conductance);
                if(idx2!=-1) A.add(idx2,idx2,conductance); // This won't be called if idx2 is GND
                if(idx1!=-1&&idx2!=-1) 
                {
                    A.add(idx1,idx2,-conductance);
                    A.add(idx2,idx1,-conductance);
                }
            }
            else if(type=="CURRENTSOURCE") 
            {
                const CurrentSource* is=static_cast<const CurrentSource*>(el.get());
                double instantaneousCurrent=is->getInstantaneousValue(currentTime);
                int idx1=(is->getNode1()=="GND")?-1:nodeToIndex.at(is->getNode1());
                int idx2=(is->getNode2()=="GND")?-1:nodeToIndex.at(is->getNode2());
                if(idx1!=-1) b[idx1]-=instantaneousCurrent;
                if(idx2!=-1) b[idx2]+=instantaneousCurrent;
            }
            else if(type=="VOLTAGESOURCE") 
            {
                const VoltageSource* vs=static_cast<const VoltageSource*>(el.get());
                double instantaneousVoltage=vs->getInstantaneousValue(currentTime);
                int idx1=(vs->getNode1()=="GND")?-1:nodeToIndex.at(vs->getNode1());
                int idx2=(vs->getNode2()=="GND")?-1:nodeToIndex.at(vs->getNode2());
                int branchIdx=voltageSourceNameToCurrentIndex.at(vs->getName());
                // KCL equations (rows corresponding to nodes)
                if(idx1!=-1) A.add(idx1,branchIdx,1.0);
                if(idx2!=-1) A.add(idx2,branchIdx,-1.0);
                // Voltage constraint equation (row corresponding to branch current)
                if(idx1!=-1) A.add(branchIdx,idx1,1.0);
                if(idx2!=-1) A.add(branchIdx,idx2,-1.0);
                b[branchIdx]+=instantaneousVoltage;
            }
            else // Capacitors and Inductors use their stampTransient method for companion models
                // They need the prev_voltages and prev_branch_currents from the class members.
                el->stampTransient(A,b,nodeToIndex,voltageSourceNameToCurrentIndex,actual_dt,currentTime,prev_voltages,prev_branch_currents);
        }
        vector<double> current_solution;
        try 
        {
            current_solution=A.solveGaussianElimination(b);
        }
        catch(const CircuitError& e) 
        {
            cerr<<"Error at Time "<<currentTime<<"s: "<<e.what()<<"\n";
            last_analysis_type="NONE";
            break; // Break or handle gracefully if solution fails
        }
        // Update class member prev_voltages and prev_branch_currents for the *next* time step
        // and for the .print command (which always prints the last computed step).
        for(int i=0; i<numNonGNDNodes; ++i)
            prev_voltages[i]=current_solution[i];
        // indexing for prev_branch_currents
        for(const auto& pair:voltageSourceNameToCurrentIndex) // pair.second holds the absolute MNA matrix index for this branch current variable
            prev_branch_currents[pair.second]=current_solution[pair.second];
        // Print results if current time is >= tstart
        if(currentTime>=tstart-1e-9) 
        {
            cout<<fixed<<setprecision(6)<<setw(15)<<currentTime;
            for(const auto& pair:nodeToIndex) 
                if(pair.second!=-1)
                    cout<<setw(15)<<prev_voltages[pair.second];
            for(const auto& el:elements) 
            {
                string type=toUpper(el->getType());
                if(type=="VOLTAGESOURCE"||type=="INDUCTOR") 
                {
                    auto it=voltageSourceNameToCurrentIndex.find(el->getName());
                    if(it!=voltageSourceNameToCurrentIndex.end())
                        cout<<setw(15)<<prev_branch_currents[it->second];
                    else
                        cout<<setw(15)<<"N/A";
                }
            }
            cout<<"\n";
        }
        // Update internal states of reactive elements for the next time step.
        // They need the *current* solution, which is now in `prev_voltages` and `prev_branch_currents`.
        for(auto& el:elements) 
        {
            // Pass the class members prev_voltages and prev_branch_currents as the 'current_voltages'/'current_branch_currents'
            // parameters to the updateTransientState methods of individual elements.
            el->updateTransientState(prev_voltages,prev_branch_currents,nodeToIndex,voltageSourceNameToCurrentIndex,actual_dt);
        }
        // Increment current time
        if(abs(currentTime-tstop)<1e-10)
            break;
        currentTime+=actual_dt;
        if(currentTime>tstop+1e-9&&abs(currentTime-tstop)>1e-10) // Check if we overshot, adjust
            currentTime=tstop;
    }
    cout<<"--- Transient Analysis Complete ---\n";
}

// Prints specific results (voltages or currents) from the last analysis
void Circuit::printResults(const vector<string>& whatToPrint) const 
{
    if(last_analysis_type=="NONE") 
    {
        cout<<"No analysis has been performed yet to print results, or the last analysis failed."<<endl;
        return;
    }
    if(whatToPrint.empty()) 
    {
        cout<<"Usage: .print <V(node)> <I(element)> ..."<<endl;
        return;
    }
    cout<<"\n--- Printed Results ("<<last_analysis_type<<" Analysis) ---\n";
    // Print header
    for(const auto& item:whatToPrint)
        cout<<setw(20)<<item;
    cout<<"\n";
    // Print values
    for(const auto& item:whatToPrint) 
    {
        string upperItem=toUpper(item);
        smatch match;
        // Try to match V(node)
        regex voltage_regex(R"(V\(([A-Z0-9_]+)\))");
        if(regex_match(upperItem,match,voltage_regex)) 
        {
            string nodeName=match[1].str();
            auto it=nodeToIndex.find(nodeName);
            if(it!=nodeToIndex.end()) 
            {
                int nodeIdx=it->second;
                if(nodeIdx==-1) // GND node
                    cout<<setw(20)<<fixed<<setprecision(4)<<0.0<<"V";
                else 
                {
                    if(last_analysis_type=="DC") 
                    {
                        if(nodeIdx<last_dc_solution.size())
                            cout<<setw(20)<<fixed<<setprecision(4)<<last_dc_solution[nodeIdx]<<"V";
                        else
                            cout<<setw(20)<<"N/A (DC)";
                    }
                    else if(last_analysis_type=="AC") 
                    {
                        string key="V("+nodeName+")";
                        auto ac_it=last_ac_solution_phasors.find(key);
                        if(ac_it!=last_ac_solution_phasors.end()) 
                        {
                            Complex voltage=ac_it->second;
                            cout<<setw(20)<<fixed<<setprecision(4)<<voltage.magnitude()<<"V<"<<voltage.angleDegrees()<<"°";
                        }
                        else
                            cout<<setw(20)<<"N/A (AC)";
                    }
                    else if(last_analysis_type=="TRANSIENT") 
                    {
                        if(nodeIdx<prev_voltages.size()) // prev_voltages holds the last computed transient step
                            cout<<setw(20)<<fixed<<setprecision(4)<<prev_voltages[nodeIdx]<<"V";
                        else
                            cout<<setw(20)<<"N/A (Tran)";
                    }
                }
            }
            else
                cout<<setw(20)<<"Node not found";
        }
        // Try to match I(element)
        else if(regex_match(upperItem,match,regex(R"(I\(([A-Z][A-Z0-9_]*)\))"))) 
        {
            string elementName=match[1].str();
            Element* targetElement=nullptr;
            for(const auto& el:elements)
                if(toUpper(el->getName())==elementName) 
                {
                    targetElement=el.get();
                    break;
                }

            if(!targetElement) 
            {
                cout<<setw(20)<<"Element not found";
                continue;
            }
            string elemType=toUpper(targetElement->getType());
            int idx1=(toUpper(targetElement->getNode1())=="GND")?-1:nodeToIndex.at(toUpper(targetElement->getNode1()));
            int idx2=(toUpper(targetElement->getNode2())=="GND")?-1:nodeToIndex.at(toUpper(targetElement->getNode2()));
            // For R, C, I_source, current is derived from node voltages
            // For V_source, L, current is a branch variable
            if(elemType=="VOLTAGESOURCE"||elemType=="INDUCTOR") 
            {
                auto it=voltageSourceNameToCurrentIndex.find(elementName);
                if(it!=voltageSourceNameToCurrentIndex.end()) 
                {
                    int branchIdx=it->second;
                    if(last_analysis_type=="DC") 
                    {
                        if(branchIdx<last_dc_solution.size())
                            cout<<setw(20)<<fixed<<setprecision(4)<<last_dc_solution[branchIdx]<<"A";
                        else 
                            cout<<setw(20)<<"N/A (DC)";
                    }
                    else if(last_analysis_type=="AC") 
                    {
                        string key="I("+elementName+")";
                        auto ac_it=last_ac_solution_phasors.find(key);
                        if(ac_it!=last_ac_solution_phasors.end()) 
                        {
                            Complex current=ac_it->second;
                            cout<<setw(20)<<fixed<<setprecision(4)<<current.magnitude()<<"A<"<<current.angleDegrees()<<"°";
                        }
                        else
                            cout<<setw(20)<<"N/A (AC)";
                    }
                    else if(last_analysis_type=="TRANSIENT") 
                    {
                        // Ensure branchIdx is within bounds of prev_branch_currents
                        if(branchIdx<prev_branch_currents.size())
                            cout<<setw(20)<<fixed<<setprecision(4)<<prev_branch_currents[branchIdx]<<"A";
                        else
                            cout<<setw(20)<<"N/A (Tran)";
                    }
                }
                else
                    cout<<setw(20)<<"No branch current";
            }
            else if(elemType=="RESISTOR") 
            {
                const Resistor* r=static_cast<const Resistor*>(targetElement);
                if(last_analysis_type=="DC") 
                {
                    double V1_val=(idx1!=-1)?last_dc_solution[idx1]:0.0;
                    double V2_val=(idx2!=-1)?last_dc_solution[idx2]:0.0;
                    double current=(V1_val-V2_val)/r->getValue();
                    cout<<setw(20)<<fixed<<setprecision(4)<<current<<"A";
                }
                else if(last_analysis_type=="AC") 
                {
                    Complex V1_complex=(idx1!=-1)?last_ac_solution_phasors.at("V("+toUpper(targetElement->getNode1())+")"):Complex(0.0);
                    Complex V2_complex=(idx2!=-1)?last_ac_solution_phasors.at("V("+toUpper(targetElement->getNode2())+")"):Complex(0.0);
                    Complex voltage_diff=V1_complex-V2_complex;
                    Complex current=voltage_diff*r->getComplexAdmittance(last_ac_frequency);
                    cout<<setw(20)<<fixed<<setprecision(4)<<current.magnitude()<<"A<"<<current.angleDegrees()<<"°";
                }
                else if(last_analysis_type=="TRANSIENT") 
                {
                    double V1_val=(idx1!=-1)?prev_voltages[idx1]:0.0;
                    double V2_val=(idx2!=-1)?prev_voltages[idx2]:0.0;
                    double current=(V1_val-V2_val)/r->getValue();
                    cout<<setw(20)<<fixed<<setprecision(4)<<current<<"A";
                }
            }
            else if(elemType=="CAPACITOR") 
            {
                const Capacitor* c=static_cast<const Capacitor*>(targetElement);
                if(last_analysis_type=="DC") // Open circuit in DC, current is 0
                    cout<<setw(20)<<fixed<<setprecision(4)<<0.0<<"A";
                else if(last_analysis_type=="AC") 
                {
                    Complex V1_complex=(idx1!=-1)?last_ac_solution_phasors.at("V("+toUpper(targetElement->getNode1())+")"):Complex(0.0);
                    Complex V2_complex=(idx2!=-1)?last_ac_solution_phasors.at("V("+toUpper(targetElement->getNode2())+")"):Complex(0.0);
                    Complex voltage_diff=V1_complex-V2_complex;
                    Complex current=voltage_diff*c->getComplexAdmittance(last_ac_frequency);
                    cout<<setw(20)<<fixed<<setprecision(4)<<current.magnitude()<<"A<"<<current.angleDegrees()<<"°";
                }
                else if(last_analysis_type=="TRANSIENT") 
                {
                    // For transient, direct current calculation requires internal state or companion model parameters.
                    // It's not directly exposed via V(node) difference * admittance like R and AC C/L.
                    // The correct current would be the prev_current_through stored internally by the capacitor itself,
                    // but that's not exposed via the Circuit's printResults directly.
                    cout<<setw(20)<<"N/A (Tran)"; // Current not directly available from .print
                }
            }
            else if(elemType=="CURRENTSOURCE") 
            {
                const CurrentSource* is=static_cast<const CurrentSource*>(targetElement);
                if(last_analysis_type=="DC")
                    cout<<setw(20)<<fixed<<setprecision(4)<<is->getDCValue()<<"A";
                else if(last_analysis_type=="AC") 
                {
                    Complex current=is->getACPhasor(last_ac_frequency);
                    cout<<setw(20)<<fixed<<setprecision(4)<<current.magnitude()<<"A<"<<current.angleDegrees()<<"°";
                }
                else if(last_analysis_type=="TRANSIENT") 
                {
                    // Similar to capacitor, the instantaneous current for current sources during TRAN
                    // is time-varying and not stored in prev_branch_currents.
                    // It could be retrieved by calling getInstantaneousValue(last_time_point) if we stored last_time_point.
                    cout<<setw(20)<<"N/A (Tran)"; // Current not directly available from .print
                }
            }
            else
                cout<<setw(20)<<"Unsupported Type";
        }
        else
            cout<<setw(20)<<"Invalid format";
    }
    cout<<"\n--------------------------------------\n";
}

void Circuit::saveToFile(string& path,const vector<string>& commandHistory)  const
{
    ofstream file(path);
	if(!file) 
    {
		cerr<<"Error opening file "<<path<<" for writing."<<endl;
		return;
	}
	for(const auto& command:commandHistory)
	{
		file<<command<<endl;
	}
    cout<<"File saving complete"<<endl;
}

vector<string> openFile(const string& path) 
{
	ifstream file(path);
	vector<string> lines;
	if(!file) 
    {
		cerr<<"Error opening file "<<path<<" for reading."<<endl;
		return lines; // Return empty vector on error
	}
	string line;
	while(getline(file,line)) {
		lines.push_back(line);
	}
	return lines;
}