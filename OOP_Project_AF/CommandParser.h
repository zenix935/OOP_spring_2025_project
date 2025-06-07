#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <string>
#include <vector>
#include <sstream> // For istringstream
#include <algorithm> // For transform
#include <cctype> // For toupper

// Note: The toUpper function is defined in Element.h
// and included indirectly via Circuit.h or directly where needed.
// It is removed from here to prevent multiple definition errors.

using namespace std;

// Function to split a string by whitespace
inline vector<string> splitString(const string& s) {
    vector<string> parts;
    istringstream iss(s);
    string part;
    while(iss>>part) {
        parts.push_back(part);
    }
    return parts;
}

#endif // COMMAND_PARSER_H