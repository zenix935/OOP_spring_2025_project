#pragma once
#include <vector>
#include "Node.h"
#include "Component.h"
#include "Resistor.h"
class Controler
{
private:
	vector<Component*> components;
	vector<Node*> nodes;
};