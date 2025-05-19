#pragma once
#include "Controller.h"
class View
{
public:
	static void run()
	{
		string input;
		while(true)
		{
			getline(cin,input);
			if(input=="exit")
				break;
		}
	}
};