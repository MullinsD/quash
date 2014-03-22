#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <cstring>
#include <errno.h>

using namespace std;

void changeDir( vector<string> test );
void set( vector<string> command );
vector<string> buildCommand( string arg );

int main( int argc, char **argv, char **envp )
{
   char ** test;
   vector<string> command;
   string commandLine;

   while( getline( cin, commandLine ) )
   {
       command = buildCommand( commandLine );

       if( command[ 0 ] == "quit" )
       {
            exit( 0 );
       }
       if( command[ 0 ] == "cd" )
       {
	    changeDir( command );
       }
       if( command[ 0 ] == "set" )
       {
            set( command );
       }
   }
  
}

void changeDir( vector<string> test )
{
    if( test.size() == 1 )
    {
        if( chdir( getenv( "HOME" ) ) < 0 )
        {
            cout << "YOU SUCK" << endl;
        }
        else
        {
            system( "ls" ); 
        }
    }
}

void set( vector<string> command )
{
        string newPath;

	if( command.size() == 1 )
	{       // If there are no arguments, tell user how to use.
		cout << "When using set, you must enter an enviroment variable to edit, in this case, HOME= or PATH=\n";
                return;
	}

	newPath = command[1].substr(5);

	if(strncmp(command[1].c_str(), "HOME=", 5) == 0)
        {
	        // If the argument is HOME=
	       	if(setenv("HOME", newPath.c_str(), 1) == -1)
                {
			// Return error if unsucessful.
			cout << "HOME was not set, due to error #" << errno << "\n";	
                }
                cout << getenv( "HOME" );
        }
	
	else if(strncmp(command[1].c_str(), "PATH=", 5) == 0)
	{
	        // If the argument is PATH=
		if(setenv("PATH", newPath.c_str(), 1) == -1)
		{
			// Return error if unsuccessful.
			cout << "PATH was not set, due to error #" << errno << "\n";
		}
                cout << getenv( "PATH" );
	}

	else
	{
		// If there was an argument entered but it wasn't PATH= or HOME=, error.
		cout << "The argument you entered isn't an actual variable, use HOME= or PATH=\n";
	}
}

int fileExists(std::string theFile)
{ /* Checks to see if a file exists using the access system call, returns 0 if it does,
     returns -1 if it does not, and prints an error message. */
	if(access(theFile.c_str(), F_OK) != 0)
	{
		cout << "File " << theFile << " does not exist.\n Please enter a valid filename.\n";
		return -1;
	}

	return 0;
}

vector<string> buildCommand( string arg )
{
    string iterator;
    stringstream input;
    vector<string> build;

    input << arg;
    while( input >> iterator )
    {
        build.push_back( iterator );
    }

   return build;
}
       	
