#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
#include <stdexcept>
#include <string>
// Base class for custom circuit errors
class CircuitError : public std::runtime_error 
{
public:
    explicit CircuitError(const std::string& message): std::runtime_error("Circuit Error: "+message) {}
};

// Specific exception for when an element name is duplicated
class DuplicateNameError : public CircuitError 
{
public:
    explicit DuplicateNameError(const std::string& name,const std::string& type)
        : CircuitError("'"+name+"' ("+type+") already exists. Please choose a unique name.") {}
};

// Specific exception for when an element is not found
class ElementNotFoundError : public CircuitError 
{
public:
    explicit ElementNotFoundError(const std::string& name) : CircuitError("Element '"+name+"' not found in the circuit.") {}
};

// Specific exception for when a node is not found
class NodeNotFoundError : public CircuitError 
{
public:
    explicit NodeNotFoundError(const std::string& name) : CircuitError("Node '"+name+"' not found in the circuit.") {}
};

// Specific exception for unsupported element types
class UnsupportedTypeError : public CircuitError
{
public:
    explicit UnsupportedTypeError(const std::string& type) : CircuitError("Unsupported element type: '"+type+"'.") {}
};

// Specific exception for invalid value strings (e.g., non-numeric, bad unit)
class InvalidValueError : public CircuitError 
{
public:
    explicit InvalidValueError(const std::string& message) : CircuitError("Invalid value: "+message) {}
};

// Specific exception for issues during matrix operations (e.g., singular matrix)
class MatrixError : public CircuitError 
{
public:
    explicit MatrixError(const std::string& message) : CircuitError("Matrix Error: "+message) {}
};

// Specific exception for command parsing errors
class SyntaxError : public CircuitError 
{
public:
    explicit SyntaxError(const std::string& message) : CircuitError("Syntax Error: "+message) {}
};
#endif