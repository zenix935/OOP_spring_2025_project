#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <streambuf>
#include <Windows.h>
#include <filesystem>
#include <limits>
#include "Circuit.h"
#include "CommandParser.h"
#include "Exceptions.h" 
#include "Matrix.h"     
#include "Complex.h"    
using namespace std;
void displayUsage() 
{
    cout<<"\n--- Usage Examples ---"<<endl;
    cout<<"add <type> <name> <node1> <node2> <value>"<<endl;
    cout<<"  Supported types: R (Resistor), C (Capacitor), L (Inductor)"<<endl;
    cout<<"  e.g., add R1 N1 N2 1000"<<endl;
    cout<<"  e.g., add C1 N2 GND 1u"<<endl;
    cout<<"  e.g., add L1 N3 N4 10m"<<endl;
    cout<<"addsource <type> <name> <node1> <node2> <DC_value>"<<endl;
    cout<<"  e.g., addsource V1 IN GND 5"<<endl; // DC Voltage Source
    cout<<"  e.g., addsource I1 N1 N2 0.01"<<endl; // DC Current Source
    cout<<"addsource <type> <name> <node1> <node2> AC <AC_magnitude> <AC_phase_degrees> <AC_frequency>"<<endl;
    cout<<"  e.g., addsource V_AC N1 GND AC 1 0 60"<<endl; // AC Voltage Source (1V, 0 deg, 60Hz)
    cout<<"  e.g., addsource I_AC N2 N3 AC 0.001 90 1k"<<endl; // AC Current Source (1mA, 90 deg, 1kHz)
    cout<<"delete <name>"<<endl;
    cout<<"  e.g., delete R1"<<endl;
    cout<<".rename node <old_name> <new_name>"<<endl;
    cout<<"  e.g., .rename node N1 INPUT"<<endl;
    cout<<".nodes"<<endl;
    cout<<".list [component_type]"<<endl;
    cout<<"  e.g., .list"<<endl;
    cout<<"  e.g., .list resistor"<<endl;
    cout<<".mna"<<endl; // Build and print DC MNA matrix/vector
    cout<<".dc"<<endl; // Perform linear DC analysis
    cout<<".dc <source_name> <start_value> <end_value> <num_points>"<<endl; // DC Sweep
    cout<<"  e.g., .dc V1 0 10 100"<<endl; // Sweep V1 from 0 to 10V 100 points
    cout<<".tran <tstep> <tstop> [<tstart>] [<tmaxstep>]"<<endl;
    cout<<"  e.g., .tran 1u 1m"<<endl; // Simulate from 0 to 1ms with 1us steps
    cout<<"  e.g., .tran 10n 100n 50n"<<endl; // Simulate start printing at 50ns
    cout<<".print <V(node_name)> <I(element_name)> ..."<<endl; // Print specific results
    cout<<"  e.g., .print V(N1) I(R1)"<<endl;
    cout<<"OPEN: opens a file"<<endl;
    cout<<"SAVE: saves a circuit on device"<<endl;
    cout<<"exit"<<endl;
    cout<<"----------------------\n"<<endl;
}
void run(vector<string>parts,string commandLine,string cmd,Circuit& circuit,vector<string>& commandHistory)
{
    try 
    {
        if(cmd=="ADD") 
        {
            if(parts.size()==6) 
            { // For R, C, L
                circuit.addElement(parts[1],parts[2],parts[3],parts[4],parts[5]);
                commandHistory.push_back(commandLine);
            }
            else
                throw SyntaxError("Invalid 'add' command format. See usage. (Only R, C, L supported)");
        }
        else if(cmd=="ADDSOURCE") 
        { // New command for sources
            if(parts.size()==6) 
            { // DC Source: addsource V1 N1 GND 5
                circuit.addSource(parts[1],parts[2],parts[3],parts[4],parts[5]);
                commandHistory.push_back(commandLine);
            }
            else if(parts.size()==9&&toUpper(parts[5])=="AC") 
            { // AC Source: addsource V_AC N1 GND AC 1 0 60
                circuit.addSource(parts[1],parts[2],parts[3],parts[4],parts[6],parts[7],parts[8]);
                commandHistory.push_back(commandLine);
            }
            else
                throw SyntaxError("Invalid 'addsource' command format. See usage.");
        }
        else if(cmd=="DELETE") 
        {
            if(parts.size()!=2)
                throw SyntaxError("delete <name>");
            circuit.deleteElement(parts[1]);
            commandHistory.push_back(commandLine);
        }
        else if(cmd==".RENAME") 
        {
            if(parts.size()!=4||toUpper(parts[1])!="NODE")
                throw SyntaxError(".rename node <old_name> <new_name>");
            circuit.renameNode(parts[2],parts[3]);
            commandHistory.push_back(commandLine);
        }
        else if(cmd==".NODES") 
        {
            if(parts.size()!=1)
                throw SyntaxError(".nodes (no arguments)");
            circuit.listNodes();
        }
        else if(cmd==".LIST") 
        {
            if(parts.size()==1)
                circuit.listElements();
            else if(parts.size()==2)
                circuit.listElements(parts[1]);
            else
                throw SyntaxError(".list [component_type]");
        }
        else if(cmd==".MNA") 
        { // Build and print DC MNA matrix/vector
            if(parts.size()!=1)
                throw SyntaxError(".mna (no arguments)");
            Matrix<double> A(0,0);
            vector<double> b;
            circuit.buildMNAMatrix(A,b);
            A.print("DC MNA Matrix A");
            cout<<"Vector b: ";
            for(double val:b)
                cout<<fixed<<setprecision(4)<<val<<" ";
            cout<<endl;
        }
        else if(cmd==".DC") 
        { // Perform linear DC analysis OR DC Sweep
            if(parts.size()==1) // Simple .dc
                circuit.solveLinearDC();
            else if(parts.size()==5) 
            { // DC Sweep: .dc <source_name> <start_value> <end_value> <num_points>
                string sourceName=parts[1];
                double startVal=Element::parseValue(parts[2]); 
                double endVal=Element::parseValue(parts[3]); 
                int numPoints=static_cast<int>(Element::parseValue(parts[4]));
                circuit.solveDCSweep(sourceName,startVal,endVal,numPoints);
            }
            else 
                throw SyntaxError(".dc OR .dc <source_name> <start_value> <end_value> <num_points>");
        }
        else if(cmd==".AC") 
        { // Perform AC analysis
            if(parts.size()!=5)
                throw SyntaxError(".ac <sweep_type> <start_freq> <end_freq> <num_points>");
            string sweepType=toUpper(parts[1]);
            double startFreq=Element::parseValue(parts[2]); 
            double endFreq=Element::parseValue(parts[3]); 
            int numPoints=static_cast<int>(Element::parseValue(parts[4]));
            circuit.solveAC(startFreq,endFreq,numPoints,sweepType);
        }
        else if(cmd==".TRAN") 
        { // Perform Transient analysis
            if(parts.size()<3||parts.size()>5)
                throw SyntaxError(".tran <tstep> <tstop> [<tstart>] [<tmaxstep>]");
            double tstep=Element::parseValue(parts[1]); 
            double tstop=Element::parseValue(parts[2]); 
            double tstart=0.0;
            double tmaxstep=0.0; 
            if(parts.size()>=4)
                tstart=Element::parseValue(parts[3]); // Changed type to Element
            if(parts.size()==5)
                tmaxstep=Element::parseValue(parts[4]); // Changed type to Element
            circuit.solveTransient(tstep,tstop,tstart,tmaxstep);
        }
        else if(cmd==".PRINT") 
        { // Print specific results
            if(parts.size()<2)
                throw SyntaxError(".print <V(node_name)> <I(element_name)> ...");
            vector<string> itemsToPrint(parts.begin()+1,parts.end());
            circuit.printResults(itemsToPrint);
        }
        else if(cmd=="SAVE")
        {
            cout<<"Give a Path to a folder to save the circuit> if 'enter' :(Default: C:/MINE/Uni/OOP_save)"<<endl;
            string path;
            getline(cin,path);
            if(path.empty())
                path="C:/MINE/Uni/OOP_save"; // Default path
            string name;
			cout<<"Give a name for the file> if 'enter' :(Default: circuit.txt)"<<endl;
            getline(cin,name);
			path+="/"+name;
            circuit.saveToFile(path,commandHistory);
        }
        else
            cout<<"Error: Unknown command. Type 'exit' to quit or see usage above."<<endl;
    }
    catch(const CircuitError& e)
    {
        cerr<<"Error: "<<e.what()<<endl;
    }
    catch(const exception& e) 
    {
        cerr<<"An unexpected error occurred: "<<e.what()<<endl;
    }
}
int main() 
{
	Circuit circuit; 
    string commandLine;
    cout<<"Welcome to the C++ Circuit Simulator (Linear DC, AC & Transient)!"<<endl;
    displayUsage();
	vector<string> commandHistory;
    while(true) 
    {   
        bool flag=false;
        cout<<"> ";
        getline(cin,commandLine);
        if(commandLine.empty())
            continue;
        vector<string> parts=splitString(commandLine);
        string cmd=toUpper(parts[0]);
        if(cmd=="EXIT")
            break;
        if(cmd=="OPEN")
        {
            string temp;
            vector<string> filenames;
            for(const auto& entry:std::filesystem::directory_iterator("C:/MINE/Uni/OOP_save"))
                filenames.push_back(entry.path().filename().string());
            cout<<"Choose one file or typr '0' for your custome path:"<<endl;
            for(int i=0;i<filenames.size();i++)
                cout<<i+1<<". "<<filenames[i]<<endl;
            cin>>temp;
            int num=stoi(temp);
            if(num!=0&&num>=1&&num<=filenames.size())
            {
                circuit=Circuit(); // Reset circuit before loading
                string path="C:/MINE/Uni/OOP_save/"+filenames[num-1];
                ifstream file(path);
                string line;
                vector<string> lines;
                while(getline(file,line))
                    lines.push_back(line);
                streambuf* orig_buf=cout.rdbuf();
                ofstream null_tream("NULL");
                cout.rdbuf(null_tream.rdbuf());
                commandHistory.clear();
                for(const auto& l:lines)
                {
                    commandLine=l;
                    parts=splitString(commandLine);
                    cmd=toUpper(parts[0]);
                    run(parts,commandLine,cmd,circuit,commandHistory);
                }
                cout.rdbuf(orig_buf);
				cout<<"File oppened successfully"<<endl;
                cin.ignore();
            }
        }
        else
        {
            run(parts,commandLine,cmd,circuit,commandHistory);
            cout<<endl;
        }
    }
    return 0;
}