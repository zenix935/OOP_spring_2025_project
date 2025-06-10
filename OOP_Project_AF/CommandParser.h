#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H
#include <string>
#include <vector>
#include <sstream> 
#include <algorithm> 
#include <cctype>
using namespace std;
// Function to split a string by whitespace
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