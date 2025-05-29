#pragma once
#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include <stdexcept>
#include <string>
using namespace std;
class CircuitError : public std::runtime_error 
{
public:
    explicit CircuitError(const std::string& message) : std::runtime_error(message) {}
};
class DuplicateNameError : public CircuitError 
{
public:
    explicit DuplicateNameError(const std::string& name,const std::string& type)
        : CircuitError("Error: "+type+" with name '"+name+"' already exists.") {}
};
class ElementNotFoundError : public CircuitError 
{
public:
    explicit ElementNotFoundError(const std::string& name)
        : CircuitError("Error: Element with name '"+name+"' not found.") {}
};
class NodeNotFoundError : public CircuitError 
{
public:
    explicit NodeNotFoundError(const std::string& name)
        : CircuitError("Error: Node '"+name+"' not found.") {}
};
class SyntaxError : public CircuitError 
{
public:
    explicit SyntaxError(const std::string& expected)
        : CircuitError("Error: Invalid command syntax. Expected: "+expected) {}
};
class InvalidValueError : public CircuitError 
{
public:
    explicit InvalidValueError(const std::string& message)
        : CircuitError("Error: Invalid value provided. "+message) {}
};
class UnsupportedTypeError : public CircuitError 
{
public:
    explicit UnsupportedTypeError(const std::string& message)
        : CircuitError("Error: "+message) {}
};
#endif