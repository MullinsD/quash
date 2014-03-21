// set.cpp
#include <iostream>
void set(char** arg)
{

	if(!arg[1])
	{       // If there are no arguments, tell user how to use.
		cerr << "When using set, you must enter an enviroment variable to edit, in this case, HOME= or PATH=\n";
		return;
	}

	else if(strncmp(arg[1], "HOME=", 5) == 0)
	        // If the argument is HOME=
	       	if(setenv("HOME", &arg[1][5], 1) == -1)
			// Return error if unsucessful.
			cerr << "HOME was not set, due to error #" << errno << "\n";	
	
	else if(strncmp(arg[1], "PATH=", 5) == 0)
	        // If the argument is PATH=
		if(setenv("PATH", &arg[1][5], 1) == -1)
			// Return error if unsuccessful.
			cerr << "PATH was not set, due to error #" << errno << "\n";
	
	else
		// If there was an argument entered but it wasn't PATH= or HOME=, error.
		cerr <"The argument you entered isn't an actual variable, use HOME= or PATH=\n";
}
