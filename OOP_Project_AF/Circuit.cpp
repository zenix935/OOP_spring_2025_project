#include "Circuit.h"
#include "CommandParser.h"
#include <iostream>
#include <algorithm>
#include <map>
#include <cmath>
#include <iomanip>
bool Circuit::hasElement(const string& name) const
{
	string upperName=toUpper(name);
	for(const auto& element:elements)
	{
		if(toUpper(element->getName())==upperName)
			return true;
	}
	return false;
}
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
    // Assign unique indices only for independent voltage sources
    int currentBranchIndex=nodes.size()-1; // Start after node voltage indices
    for(const auto& el:elements) 
    {
        string type=toUpper(el->getType());
        if(type=="VOLTAGESOURCE"||type=="INDUCTOR") // Only independent voltage sources
            voltageSourceNameToCurrentIndex[el->getName()]=currentBranchIndex++;
    }
}
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
Circuit::Circuit() 
{
    nodes.insert("GND"); // Initialize with a default ground node
    updateNodesAndBranchCurrentIndices(); // Initial map build
}
// Add an element to the circuit (for R, C, L)
void Circuit::addElement(const string& type,const string& name,const string& node1,const string& node2,const string valueStr)
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
// Overload for VoltageSource/CurrentSource (DC or AC)
// Now takes 3 value parameters for AC sources (Mag, Phase, Freq)
void Circuit::addSource(const string& type,const string& name,const string& node1,const string& node2,
    const string& val1,const string& val2,const string& val3) {
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
            newElement=std::make_unique<CurrentSource>(name,node1,node2,val1);
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
        throw DuplicateNameError(newName,"Node name")
    // Update nodes in elements
    for(auto& el:elements) 
    {
        if(toUpper(el->getNode1())==upperOldName)
            el->setNode1(upperNewName);
        if(toUpper(el->getNode2())==upperOldName)
            el->setNode2(upperNewName)
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
            if(toUpper(el->getType()).find(upperComponentType)!=string::npos) 
            {
                cout<<"- "<<el->toString()<<endl;
                foundAny=true;
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
    cout<<endl<<"--- Building MNA Matrix (Linear DC) ---"<<endl;
    cout<<"Number of non-GND nodes: "<<numNonGNDNodes<<endl;
    cout<<"Number of elements adding branch currents: "<<voltageSourceNameToCurrentIndex.size()<<endl;
    cout<<"Total MNA matrix size: "<<mnaSize<<"x"<<mnaSize<<endl;
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
        // No other element types are supported for linear DC analysis.
    }
    cout<<"--- MNA Matrix Building Complete ---"<<endl;
}
// Solves the circuit for linear DC analysis using Gaussian Elimination
void Circuit::solveLinearDC() 
{
    if(elements.empty()) 
    {
        cout<<"Circuit is empty. Nothing to analyze."<<endl;
        return;
    }
    Matrix<double> A(0,0); // Use Matrix<double>
    vector<double> b;
    try 
    {
        // Build the linear MNA matrix (A) and RHS vector (b)
        buildMNAMatrix(A,b);
        // Solve the linear system A * x = b using Gaussian Elimination
        vector<double> solutionVector=A.solveGaussianElimination(b);
        cout<<endl<<"--- DC Analysis Results (Linear Circuit) ---"<<endl;
        // Print node voltages
        for(const auto& pair:nodeToIndex) 
        {
            if(pair.second!=-1) // Not GND
                cout<<"Node "<<pair.first<<": "<<fixed<<setprecision(6)<<solutionVector[pair.second]<<"V"<<endl;
            else {
                cout<<"Node GND: 0.000000V"<<endl;
            }
        }
        // Print branch currents for independent voltage sources and inductors
        for(const auto& pair:voltageSourceNameToCurrentIndex)
            cout<<"Current through "<<pair.first<<": "<<fixed<<setprecision(6)<<solutionVector[pair.second]<<"A"<<endl;
        cout<<"--------------------------------------"<<endl;
    }
    catch(const CircuitError& e) 
    {
        cerr<<"DC Analysis Error: "<<e.what()<<endl;
    }
    catch(const exception& e) 
    {
        cerr<<"An unexpected error occurred during DC analysis: "<<e.what()<<endl;
    }
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
    cout<<endl<<"--- Building AC MNA Matrix (Frequency: "<<frequency<<" Hz) ---"<<endl;
    cout<<"Number of non-GND nodes: "<<numNonGNDNodes<<endl;
    cout<<"Number of elements adding branch currents: "<<voltageSourceNameToCurrentIndex.size()<<endl;
    cout<<"Total MNA matrix size: "<<mnaSize<<"x"<<mnaSize<<endl;
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
        { // Inductors handled separately below
            if(idx1!=-1)
                A_ac.add(idx1,idx1,admittance);
            if(idx2!=-1)
                A_ac.add(idx2,idx2,admittance);
            if(idx1!=-1&&idx2!=-1) 
            {
                A_ac.add(idx1,idx2,-admittance);
                A_ac.add(idx2,idx1,-admittance);
            }
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
            Complex impedance=Complex(1.0)/admittance; // Impedance of inductor
            if(idx1!=-1)
                A_ac.add(branchIdx,idx1,Complex(1.0));
            if(idx2!=-1)
                A_ac.add(branchIdx,idx2,Complex(-1.0));
            A_ac.add(branchIdx,branchIdx,-impedance); // Coefficient for inductor current variable
            // b[branchIdx] remains 0 for this constraint
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
        // No other element types are supported for AC analysis in this simplified version.
    }
    cout<<"--- AC MNA Matrix Building Complete ---"<<endl;
}
// Performs AC analysis over a frequency range
void Circuit::solveAC(double startFreq,double endFreq,int numPoints,const string& sweepType) 
{
    if(elements.empty()) 
    {
        cout<<"Circuit is empty. Nothing to analyze."<<endl;
        return;
    }
    if(numPoints<=0)
        throw CircuitError("Number of points for AC sweep must be positive.");
    if(startFreq<=0||endFreq<=0)
        throw CircuitError("Start and end frequencies must be positive for AC analysis.");
    if(startFreq>endFreq)
        throw CircuitError("Start frequency cannot be greater than end frequency.");
    cout<<endl<<"--- Starting AC Analysis ---"<<endl;
    cout<<"Frequency range: "<<startFreq<<" Hz to "<<endFreq<<" Hz"<<endl;
    cout<<"Number of points: "<<numPoints<<", Sweep type: "<<sweepType<<endl;
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
    cout<<endl;
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
        try 
        {
            buildACMNAMatrix(A_ac,b_ac,currentFreq);
            vector<Complex> solutionVector_ac=A_ac.solveGaussianElimination(b_ac);
            // Print results for current frequency
            cout<<fixed<<setprecision(4)<<setw(15)<<currentFreq;
            for(const auto& pair:nodeToIndex)
                if(pair.second!=-1) 
                {
                    Complex nodeVoltage=solutionVector_ac[pair.second];
                    cout<<setw(15)<<nodeVoltage.mag()<<setw(15)<<nodeVoltage.phaseDeg();
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
                        cout<<setw(15)<<branchCurrent.mag()<<setw(15)<<branchCurrent.phaseDeg();
                    }
                    else // This case should ideally not happen if updateNodesAndBranchCurrentIndices is correct
                        cout<<setw(15)<<"N/A"<<setw(15)<<"N/A";
                }
            }
            cout<<endl;
        }
        catch(const CircuitError& e) 
        {
            cerr<<"Error at "<<currentFreq<<" Hz: "<<e.what()<<endl;
            // Print empty results or error indication for this frequency point
            cout<<fixed<<setprecision(4)<<setw(15)<<currentFreq;
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
            cout<<endl;
        }
    }
    cout<<"--- AC Analysis Complete ---"<<endl;
}
// Performs Transient analysis
void Circuit::solveTransient(double tstep,double tstop,double tstart,double tmaxstep) 
{
    if(elements.empty()) 
    {
        cout<<"Circuit is empty. Nothing to analyze."<<endl;
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
        cerr<<"Warning: Tmaxstep ("<<tmaxstep<<") is less than Tstep ("<<tstep<<"). Setting Tmaxstep to Tstep."<<endl;
        tmaxstep=tstep;
    }
    cout<<endl<<"--- Starting Transient Analysis ---"<<endl;
    cout<<"Tstep: "<<tstep<<"s, Tstop: "<<tstop<<"s, Tstart: "<<tstart<<"s, Tmaxstep: "<<tmaxstep<<"s"<<endl;
    // Initialize previous states of reactive elements
    for(auto& el:elements)
        el->initializeTransientState();
    // Determine the size of the MNA matrix for transient analysis
    int numNonGNDNodes=nodes.size()-1;
    int mnaSize=numNonGNDNodes+voltageSourceNameToCurrentIndex.size();
    // Initialize prev_voltages and prev_branch_currents vectors
    // These will hold the solution from the previous time step.
    prev_voltages.assign(numNonGNDNodes,0.0);
    prev_branch_currents.assign(voltageSourceNameToCurrentIndex.size(),0.0);
    // Prepare results header
    cout<<setw(15)<<"Time (s)";
    for(const auto& pair:nodeToIndex)
        if(pair.second!=-1)
            std::cout<<std::setw(15)<<"V("+pair.first+")";
    for(const auto& el:elements) 
    {
        string type=toUpper(el->getType());
        if(type=="VOLTAGESOURCE"||type=="INDUCTOR")
            cout<<setw(15)<<"I("+el->getName()+")";
    }
    cout<<endl;
    double currentTime=0.0;
    // The actual time step used for integration might vary if tmaxstep is used for adaptive stepping.
    // For simplicity, we'll use tstep as the fixed integration step for now.
    double actual_dt=tstep;
    while(currentTime<=tstop+1e-9) // Add a small epsilon to ensure tstop is included
    { 
        Matrix<double> A(0,0);
        vector<double> b;
        // Build the MNA matrix and RHS vector for the current time step
        // Each element stamps its contribution based on its companion model and previous state
        A.resize(mnaSize,mnaSize);
        b.assign(mnaSize,0.0);
        for(auto& el:elements) 
        {
            el->stampTransient(A,b,nodeToIndex,voltageSourceNameToCurrentIndex,
                actual_dt,currentTime,prev_voltages,prev_branch_currents);
        }
        vector<double> current_solution;
        try 
        {
            current_solution=A.solveGaussianElimination(b);
        }
        catch(const CircuitError& e) 
        {
            cerr<<"Error at Time "<<currentTime<<"s: "<<e.what()<<endl;
            // Break or handle gracefully if solution fails
            break;
        }
        // Extract current voltages and branch currents from the solution vector
        vector<double> current_voltages_extracted(numNonGNDNodes);
        for(int i=0; i<numNonGNDNodes; ++i)
            current_voltages_extracted[i]=current_solution[i];
        vector<double> current_branch_currents_extracted(voltageSourceNameToCurrentIndex.size());
        for(const auto& pair:voltageSourceNameToCurrentIndex)
            current_branch_currents_extracted[pair.second]=current_solution[pair.second];
        // Print results if current time is >= tstart
        if(currentTime>=tstart-1e-9) // Add epsilon for floating point comparison
        { 
            cout<<fixed<<setprecision(6)<<setw(15)<<currentTime;
            for(const auto& pair:nodeToIndex)
                if(pair.second!=-1)
                    cout<<setw(15)<<current_voltages_extracted[pair.second];
            for(const auto& el:elements) 
            {
                string type=toUpper(el->getType());
                if(type=="VOLTAGESOURCE"||type=="INDUCTOR") 
                {
                    auto it=voltageSourceNameToCurrentIndex.find(el->getName());
                    if(it!=voltageSourceNameToCurrentIndex.end())
                        cout<<setw(15)<<current_branch_currents_extracted[it->second];
                    else
                        cout<<setw(15)<<"N/A";
                }
            }
            cout<<endl;
        }
        // Update previous states for the next time step
        prev_voltages=current_voltages_extracted;
        prev_branch_currents=current_branch_currents_extracted;
        for(auto& el:elements) 
        {
            el->updateTransientState(current_voltages_extracted,current_branch_currents_extracted,
                nodeToIndex,voltageSourceNameToCurrentIndex);
        }
        currentTime+=actual_dt;
        // Consider adaptive time stepping with tmaxstep if needed in the future
        // For now, fixed time step.
    }
    cout<<"--- Transient Analysis Complete ---"<<endl;
}