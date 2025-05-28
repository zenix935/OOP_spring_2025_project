#pragma once
#ifndef ELEMENT_H
#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include "Exeptions.h"
using namespace std;
inline string toUpper(const string str) 
{
	transform(str.begin(),str.end(),str.begin(),::toupper);
	return str;
}
class Element
{
protected:
	string name;
	string node1;
	string node2;
};
#endif 
