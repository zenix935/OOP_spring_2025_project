#include <iostream>   
#include <string>
#include <vector>
#include <limits>
#include "Circuit.h"
#include "CommandParser.h"
#include "Exceptions.h"
#include "Matrix.h"
#include "Complex.h"
using namespace std;
void displayCommands()
{
	cout<<"---Available Commands---"<<endl;
	cout<<"add <type> <name> <node1> <node2> <value> "<<endl;
	cout<<" Supported types: R (Resistor), C (Capacitor), L (Inductor)"<<endl;
	cout<<" e.g.: add R1 N1 N2 100"<<endl;
	cout<<" e.g.: add C1 N2 GND 1u"<<endl;
	cout<<" e.g.: add L1 N3 N4 10m"<<endl;
	cout<<"addsource <type> <name> <node1> <node2> <DC_value>"<<endl;
	cout<<" e.g.: addsource V1 N1 GND 5"<<endl;
	cout<<" e.g.: addsource I1 N2 N3 0.01"<<endl;
	cout<<"addsource <type> <name> <node1> <node2> AC <AC_magnitude> <AC_phase_degrees> <AC_frequency>"<<endl;
	cout<<"  e.g.: addsource V_AC N1 GND AC 1 0 60"<<endl;
	cout<<"  e.g.: addsource I_AC N2 N3 AC 0.001 90 1k"<<endl;
	cout<<"delete <name>"<<endl;
	cout<<" e.g.: delete R1"<<endl;
	cout<<".rename node <old_name> <new_name>"<<endl;
	cout<<" e.g.: .rename node N1 N2"<<endl;
	cout<<".nodes"<<endl;
	cout<<".list [component_type]"<<endl;
	cout<<" e.g.: .list"<<endl;
	cout<<" e.g.: .list resistor"<<endl;
	cout<<".mna"<<endl;
	cout<<".dc"<<endl;
}
int main()  
{  
	
	return 0;  
}