#include "Element.h"
#include "CommandParser.h"
#include <map>
#include <cctype>
using namespace std;
Element::Element(const string& name,const string& node1,const string& node2,const string& type)
	: name(toUpper(name)),node1(toUpper(node1)),node2(toUpper(node2)),type(type) 
{
	if(this->name.empty()||this->node1.empty()||this->node2.empty())
		throw InvalidValueError("Element name and nodes cannot be empty.");
	if(this->node1==this->node2)
		throw InvalidValueError("Element nodes cannot be identical.");
}
double Element::parseValue(const string& valueStr) 
{
	if(valueStr.empty())
		throw InvalidValueError("Value string cannot be empty.");
	map<char,double> multipliers=
	{
		{'F',1e-15}, //Femto
		{'P',1e-12}, //Pico
		{'N',1e-9},  //Nano
		{'U', 1e-6}, // Micro
		{'M', 1e-3}, // Milli
		{'K', 1e3},  // Kilo
		{'G', 1e9},  // Giga
		{'T', 1e12}  // Tera
	};
	double value;
	char suffix='\0';
	string numPart;
	size_t i=0;
	while(i<valueStr.length()&&isdigit(valueStr[i])||valueStr[i]=='.'||(i==0&&valueStr[i]=='-')) 
	{
		numPart+=valueStr[i];
		i++;
	}
	try { value=stod(numPart); }
	catch(const invalid_argument& e){ throw InvalidValueError("Invalid numerical part in value: '"+valueStr+"'"); }
	catch(const out_of_range& e) { throw InvalidValueError("Numerical value out of range: '"+valueStr+"'"); }
	if(i<valueStr.length()) //if suffix exists
	{
		suffix=toUpper(string(1,valueStr[i]))[0];
		if(multipliers.count(suffix))
			value*=multipliers[suffix];
		else if(suffix=='M'&&i+1<valueStr.length()&&toUpper(string(1,valueStr[i+1]))[0]=='E'
			&&i+2<valueStr.length()&&toUpper(string(1,valueStr[i+2]))[0]=='G')
			value*=1e6; //Handle MEG cuase MEG and M are the same in the multipliers
		else
			throw InvalidValueError("Unrecognized unit suffix: '"+string(1,valueStr[i])+"' in value: '"+valueStr+"'.");
	}
	return value;
}
