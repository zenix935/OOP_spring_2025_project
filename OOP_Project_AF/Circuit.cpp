#include "Circuit.h"
#include <iostream>
#include <algorithm>
#include <map>
#include <cmath>
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
void Circuit::updateNodesAndVoltageSourceIndices() 
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
        if(type=="VOLTAGESOURCE") // Only independent voltage sources
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
    updateNodesAndVoltageSourceIndices(); // Initial map build
}
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
    updateNodesAndVoltageSourceIndices(); // Update nodes and rebuild map after adding
    cout<<"SUCCESS: Added "<<elements.back()->toString()<<endl;
}
void Circuit::addSource(const string& type,const string& name,const string& node1,const string& node2,const string& val1,const string& val2) 
{
    if(hasElement(name))
        throw DuplicateNameError(name,type);
    unique_ptr<Element> newElement;
    string upperType=toUpper(type);
    if(upperType=="V"||upperType=="VOLTAGESOURCE") 
    {
        if(val2.empty()) // DC Voltage Source
            newElement=make_unique<VoltageSource>(name,node1,node2,val1);
        else // AC Voltage Source (Magnitude, Phase)
            newElement=make_unique<VoltageSource>(name,node1,node2,val1,val2);
    }
    else if(upperType=="I"||upperType=="CURRENTSOURCE") 
    {
        if(val2.empty()) // DC Current Source
            newElement=make_unique<CurrentSource>(name,node1,node2,val1);
        else // AC Current Source (Magnitude, Phase)
            newElement=make_unique<CurrentSource>(name,node1,node2,val1,val2);
    }
    else
        throw UnsupportedTypeError("Unsupported source type: "+type+". Use 'add' for R, C, L.");
    elements.push_back(move(newElement));
    updateNodesAndVoltageSourceIndices();
    cout<<"SUCCESS: Added "<<elements.back()->toString()<<endl;
}
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
    updateNodesAndVoltageSourceIndices(); // Update nodes and rebuild map after deleting
    cout<<"SUCCESS: Deleted element "<<upperName<<endl;
}
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
    updateNodesAndVoltageSourceIndices(); // Rebuild the nodes set and map based on updated elements
    cout<<"SUCCESS: Node renamed from "<<upperOldName<<" to "<<upperNewName<<endl;
}
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
void Circuit::buildMNAMatrix(Matrix<double>& A,vector<double>& b) 
{
    int numNonGNDNodes=nodes.size()-1; // Ground node is not included in the matrix size for node voltages
    int mnaSize=numNonGNDNodes+voltageSourceNameToCurrentIndex.size();   // Total MNA matrix size = (number of non-GND nodes) + (number of independent voltage sources that add branch currents)
    A.resize(mnaSize,mnaSize);  // Resize matrix A and vector b
    b.assign(mnaSize,0.0); // Initialize b with zeros
    cout<<"\n--- Building MNA Matrix (Linear DC) ---\n";
    cout<<"Number of non-GND nodes: "<<numNonGNDNodes<<endl;
    cout<<"Number of independent voltage sources adding branch currents: "<<voltageSourceNameToCurrentIndex.size()<<endl;
    cout<<"Total MNA matrix size: "<<mnaSize<<"x"<<mnaSize<<endl;
    for(const auto& element:elements) 
    {
        string type=toUpper(element->getType());
        string n1=toUpper(element->getNode1());
        string n2=toUpper(element->getNode2());
        int idx1=(n1=="GND")?-1:nodeToIndex.at(n1);
        int idx2=(n2=="GND")?-1:nodeToIndex.at(n2);
        if(type=="RESISTOR") // Handle Resistors
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
        else if(type=="CURRENTSOURCE") // Handle Independent Current Sources
        {
            const CurrentSource* is=static_cast<const CurrentSource*>(element.get());
            double current=is->getDCValue(); // Use DC value
            if(idx1!=-1)
                b[idx1]-=current; // Current leaving n1 (added to RHS with negative sign)
            if(idx2!=-1)
                b[idx2]+=current; // Current entering n2 (added to RHS with positive sign)
        }
        else if(type=="VOLTAGESOURCE") // Handle Independent Voltage Sources
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
        else if(type=="CAPACITOR") // Handle Capacitors (for DC analysis, C is open circuit) 
        {
            // For DC analysis, capacitors are open circuits, so they don't contribute to MNA.
        }
        // Handle Inductors (for DC analysis, L is short circuit)
        else if(type=="INDUCTOR") {
            // For DC analysis, inductors are short circuits. This means V_n1 - V_n2 = 0.
            // A more robust implementation would add a branch current for each inductor.
            // For this basic MNA, we'll skip adding a branch current for inductors,
            // effectively treating them as ideal wires with 0V drop.
        }
        // No other element types are supported for linear DC analysis.
    }
    cout<<"--- MNA Matrix Building Complete ---\n";
}
void Circuit::solveLinearDC() 
{
    if(elements.empty()) 
    {
        cout<<"Circuit is empty. Nothing to analyze."<<endl;
        return;
    }
    Matrix<double> A(0,0);
    vector<double> b;
    try 
    {
        buildMNAMatrix(A,b); // Build the linear MNA matrix (A) and RHS vector (b)
        vector<double> solutionVector=A.solveGaussianElimination(b); // Solve the linear system A * x = b using Gaussian Elimination
        cout<<"\n--- DC Analysis Results (Linear Circuit) ---\n";
        for(const auto& pair:nodeToIndex) // Print node voltages
        {
            if(pair.second!=-1) // Not GND
                cout<<"Node "<<pair.first<<": "<<fixed<<setprecision(6)<<solutionVector[pair.second]<<endl;
            else {
                cout<<"Node GND: 0.000000V"<<endl;
            }
        }
        for(const auto& pair:voltageSourceNameToCurrentIndex) // Print branch currents for independent voltage sources
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
void Circuit::buildACMNAMatrix(Matrix<Complex>& A_ac,vector<Complex>& b_ac,double frequency) 
{
    int numNonGNDNodes=nodes.size()-1; // Ground node is not included in the matrix size for node voltages
    int mnaSize=numNonGNDNodes+voltageSourceNameToCurrentIndex.size(); // Total MNA matrix size = (number of non-GND nodes) + (number of independent voltage sources that add branch currents)
    A_ac.resize(mnaSize,mnaSize); // Resize matrix A and vector b
    b_ac.assign(mnaSize,Complex(0.0)); // Initialize b_ac with complex zeros
    cout<<"\n--- Building AC MNA Matrix (Frequency: "<<frequency<<" Hz) ---"<<endl;
    cout<<"Number of non-GND nodes: "<<numNonGNDNodes<<endl;
    cout<<"Number of independent voltage sources adding branch currents: "<<voltageSourceNameToCurrentIndex.size()<<endl;
    cout<<"Total MNA matrix size: "<<mnaSize<<"x"<<mnaSize<<endl;
    for(const auto& element:elements) 
    {
        string type=toUpper(element->getType());
        string n1=toUpper(element->getNode1());
        string n2=toUpper(element->getNode2());
        int idx1=(n1=="GND")?-1:nodeToIndex.at(n1);
        int idx2=(n2=="GND")?-1:nodeToIndex.at(n2);
        Complex admittance=element->getComplexAdmittance(frequency); // Get complex admittance for R, C
        if(type=="RESISTOR"||type=="CAPACITOR"||type=="INDUCTOR") // Handle R, C, L contributions to the MNA matrix
        {
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
        else if(type=="CURRENTSOURCE") // Handle Independent Current Sources
        {
            const CurrentSource* is=static_cast<const CurrentSource*>(element.get());
            Complex currentPhasor=is->getACPhasor(); // Use AC phasor
            if(idx1!=-1)
                b_ac[idx1]-=currentPhasor; // Current leaving n1
            if(idx2!=-1)
                b_ac[idx2]+=currentPhasor; // Current entering n2
        }
        else if(type=="VOLTAGESOURCE") // Handle Independent Voltage Sources
        {
            const VoltageSource* vs=static_cast<const VoltageSource*>(element.get());
            Complex voltagePhasor=vs->getACPhasor(); // Use AC phasor
            int branchIdx=voltageSourceNameToCurrentIndex.at(vs->getName());
            if(idx1!=-1) // KCL equation contributions (rows corresponding to nodes)
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
    cout<<setw(15)<<"Frequency (Hz)"; // Prepare results header
    for(const auto& pair:nodeToIndex)
        if(pair.second!=-1)
            cout<<setw(15)<<"V("+pair.first+") Mag"<<setw(15)<<"V("+pair.first+") Phase";
    for(const auto& pair:voltageSourceNameToCurrentIndex)
        cout<<setw(15)<<"I("+pair.first+") Mag"<<setw(15)<<"I("+pair.first+") Phase";
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
            cout<<fixed<<setprecision(4)<<setw(15)<<currentFreq; // Print results for current frequency
            for(const auto& pair:nodeToIndex)
                if(pair.second!=-1) 
                {
                    Complex nodeVoltage=solutionVector_ac[pair.second];
                    cout<<setw(15)<<nodeVoltage.mag()<<setw(15)<<nodeVoltage.phaseDeg();
                }
            for(const auto& pair:voltageSourceNameToCurrentIndex) 
            {
                Complex branchCurrent=solutionVector_ac[pair.second];
                cout<<setw(15)<<branchCurrent.mag()<<setw(15)<<branchCurrent.phaseDeg();
            }
            cout<<endl;
        }
        catch(const CircuitError& e) 
        {
            cerr<<"Error at "<<currentFreq<<" Hz: "<<e.what()<<endl;
            // Print empty results or error indication for this frequency point
            cout<<fixed<<setprecision(4)<<setw(15)<<currentFreq;
            for(size_t k=0; k<(nodes.size()-1+voltageSourceNameToCurrentIndex.size())*2; ++k)
                cout<<setw(15)<<"ERROR";
            cout<<endl;
        }
    }
    cout<<"--- AC Analysis Complete ---"<<endl;
}