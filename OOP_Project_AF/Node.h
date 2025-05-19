#pragma once
#include <string>
using namespace std;
class Node
{
private:
	string name;
	double voltage=0.0;
	bool isGround=false;
public:
	Node(string name) : name(name) {}
	double getVoltage() { return voltage; }
};

