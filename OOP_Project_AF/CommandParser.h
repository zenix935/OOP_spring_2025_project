#pragma once
#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H
#include <string>
#include <vector>
#include <sstream> 
#include <algorithm> 
using namespace std;
inline string toUpper(string s) 
{
    transform(s.begin(),s.end(),s.begin(),::toupper);
    return s;
}
inline vector<string> splitString(const string& s)
{
    vector<string> parts;
    istringstream iss(s);
    string part;
    while(iss>>part)
        parts.push_back(part);
    return parts;
}
#endif
