#include <unistd.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <cstring>
#include <errno.h>
#include <sys/wait.h>

using namespace std;

void kill(string arg);
void jobs();
void changeDir( vector<string> test );
void set( vector<string> command );
vector<string> buildCommand( string arg );
bool fileExists( string theFile );
string getPath( string );
bool stdIn(string theFile);
bool stdOut(string theFile);
void executeJob( vector<string> jerbs, char ** );

struct job
{ /* Variables to hold job info. */
	string theJob;
	int id;
	pid_t pid;
	bool isBackground;
};

vector<job> theJobs;


int main( int argc, char **argv, char **envp )
{
   char ** test;
   vector<string> command;
   string commandLine;

   while( getline( cin, commandLine ) )
   {
       command = buildCommand( commandLine );

       if( command[ 0 ] == "quit" || command[ 0 ] == "exit" )
       {
            exit( 0 );
       }
       else if( command[ 0 ] == "cd" )
       {
	    changeDir( command );
       }
       else if( command[ 0 ] == "set" )
       {
            set( command );
       }
       else
       {
        	executeJob( command, envp );
       }
   }
  
}

void executeJob( vector<string> jerbs, char ** envp )
{
	bool exists;
	char ** argv = new char*[ jerbs.size() + 1 ];
	

	for(  int i = 0; i < jerbs.size(); i++) 
	{
		argv[ i ] = new char[jerbs[i].length() + 1];
		strcpy(argv[ i], jerbs[i].c_str());
	}

	argv[ jerbs.size() ] = NULL;  


	if( jerbs[0][0] == '/' )
	{
		exists = fileExists( jerbs[0] );
	}
	else if( !strncmp( "./", jerbs[0].c_str(), 2 ) )
	{
		jerbs[0].erase( 0, 2 );

		exists = fileExists( jerbs[0] );
	}
	else
	{
		jerbs[0] = getPath( jerbs[0] );
		
		if( jerbs[0] != "" )
		{
			cout << jerbs[0] << endl;
			exists = true;
		}
		else
		{
			exists = false;
		}
	}

	if( exists )
	{
		pid_t pid = fork();
		
		if( pid	< 0 )
		{
			cout << "YOU SUCK AT FORKING" << endl;
		}
		else if( pid == 0 )
		{
			if( execve( jerbs[0].c_str(), argv, envp ) < 0 )
			{
				cout << "YOU SUCK" << endl;
			}
		}
	}
	else
	{
	cout << "FILE NOT FOUND" << endl;
	}

	wait( NULL );
}

	
string getPath( string Executable )
{
	stringstream ss( getenv( "PATH" ) );
	string tokens;
	vector<string> paths;

	while( getline( ss, tokens, ':' ) )
	{
		paths.push_back( tokens );
	}

		for(int i = 0; i < paths.size(); i++)
	{

		paths[ i ] += '/' + Executable;
		

		if( fileExists( paths[ i ] ) )
		{
			return paths[ i ];
		}
	}

	return "";
}

void changeDir( vector<string> test )
{
    if( test.size() == 1 )
    {
        if( chdir( getenv( "HOME" ) ) < 0 )
        {
            cout << "ERROR RETURNING TO HOME DIRECTORY" << endl;
        }
    }

    else if( test.size() == 2 )
    {
	if( chdir( test[1].c_str() ) < 0 )
	{
	    cout << "DIR DOES NOT EXIST" << endl;
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

void jobs()
{ /* Print all jobs. */
	if(theJobs.empty())
	{ /* There are no jobs, tell the user. */
		cout << "There are no jobs running.\n";
	}
	
	else
	{ /* Clarification for what prints below. */
		cout << "Background? / ID / PID / Doing?\n";
	}

	for(int i = 0; i < theJobs.size(); i ++)
	{ /* Print out each job, specifying what is in the background and what isn't. */
		if(theJobs[i].isBackground == true)
		{
			cout << "T " << theJobs[i].id << " " << theJobs[i].pid << " " << theJobs[i].theJob << "\n";
		}

		if(theJobs[i].isBackground == false)
		{
			cout << "F " << theJobs[i].id << " " << theJobs[i].pid << " " << theJobs[i].theJob << "\n";
		}
	}
}
 
void kill(string arg)
{ /* Kill specified process. */
	istringstream is(arg);
	int theInt;
	is >> theInt;
	if(kill(theInt, SIGINT) != 0)
	{
		cout << "Process was immortal, could not kill.\n";
	}

	else
	{
		cout << "Process " << arg << " was killed.\n";
	}
}

bool stdIn(string theFile)
{ /* Copies the file pointer to STDIN_FILENO or notifies user of mistakes. */
	FILE* inFile = fopen(theFile.c_str(), "r");
	if(fileExists(theFile) != 0)
	{ /* Check if file exists. */
		cout << "File " << theFile << " does not exist.\n Please enter a valid filename.\n";
		return false;
	}

	if(inFile == NULL)
	{ /* Check if readable. */
		cout << "File " << theFile << " is not readable.\n Please check permissions or enter another file.\n";
		return false;
	}
	
	dup2(fileno(inFile), STDIN_FILENO);
	fclose(inFile);
	return 0;
}

bool stdOut(string theFile)
{ /* Copies the file pointer to STDOUT_FILENO or notifies user of mistakes. */
	FILE* outFile = fopen(theFile.c_str(), "w");
	if(fileExists(theFile) != 0)
	{ /* Check if file exists. */
		cout << "File " << theFile << " does not exist.\n Please enter a valid filename.\n";
		return false;
	}

	if(outFile == NULL)
	{ /* Check if readable. */
		cout << "File " << theFile << " does not exist.\n Please check permissions or enter another file.\n";
		return false;
	}

	dup2(fileno(outFile), STDOUT_FILENO);
	fclose(outFile);
	return 0;
}

bool fileExists(string theFile)
{ /* Checks to see if a file exists using the access system call, returns 0 if it does,
     returns -1 if it does not, and prints an error message. */
	if(access(theFile.c_str(), F_OK) != 0)
	{
		return false;
	}

	return true;
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
       	
