#pragma once
#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H
#include <string>
#include <vector>
#include <sstream> 
#include <algorithm> 
inline std::string toUpper(std::string s) 
{
    std::transform(s.begin(),s.end(),s.begin(),::toupper);
    return s;
}
inline std::vector<std::string> splitString(const std::string& s)
{
    std::vector<std::string> parts;
    std::istringstream iss(s);
    std::string part;
    while(iss>>part)
        parts.push_back(part);
    return parts;
}
#endif
