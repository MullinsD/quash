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

struct job
{ /* Variables to hold job info. */
	bool isBackground;
	vector<string> theJob;
	string fileIn;
	string fileOut;
	int id;
	pid_t pid;
};

void kill(vector<string> arg);
void jobs();
void changeDir( vector<string> test );
void set( vector<string> command );
job * buildJob( string arg );
bool fileExists( string theFile );
string getPath( string );
bool stdIn(string theFile);
bool stdOut(string theFile);
void executeJob( job * jerb, char ** );

vector<job> theJobs;


int main( int argc, char **argv, char **envp )
{
   char ** test;
   job * jerb = new job;
   string commandLine;

   while( getline( cin, commandLine ) )
   {
       jerb = buildJob( commandLine );

       if( jerb->theJob[ 0 ] == "quit" || jerb->theJob[ 0 ] == "exit" )
       {
            exit( 0 );
       }
       else if( jerb->theJob[ 0 ] == "cd" )
       {
	    changeDir( jerb->theJob );
       }
       else if( jerb->theJob[ 0 ] == "set" )
       {
            set( jerb->theJob );
       }
       else if( jerb->theJob[ 0 ] == "jobs" )
       {
	     jobs();
       }
       else if( jerb->theJob[ 0 ] == "kill" )
       {
            kill ( jerb->theJob );
       }
       else
       {
        	executeJob( jerb, envp );
       }
   }
  
}

void executeJob( job * jerb, char ** envp )
{ /* Executes an executable, with or without arguments, while passing down
     the environment to all child processes. */
	bool exists;
	
	/* Save descriptors. */
	int theStdIn = dup(STDIN_FILENO);
	int theStdOut = dup(STDOUT_FILENO);

	if(jerb->fileIn != "")
	{ /* Redirect input. */
		stdIn(jerb->fileIn);
	}

	if(jerb->fileOut != "")
	{ /* Redirect output. */
		stdOut(jerb->fileOut);
	}

	char ** argv = new char*[ jerb->theJob.size() + 1 ];
	
	for(  int i = 0; i < jerb->theJob.size(); i++) 
	{
		argv[ i ] = new char[jerb->theJob[i].length() + 1];
		strcpy(argv[ i], jerb->theJob[i].c_str());
	}

	argv[ jerb->theJob.size() ] = NULL;  

	if( jerb->theJob[0][0] == '/' )
	{
		exists = fileExists( jerb->theJob[0] );
	}
	else if( !strncmp( "./", jerb->theJob[0].c_str(), 2 ) )
	{
		jerb->theJob[0].erase( 0, 2 );
	}
	else

	{
		jerb->theJob[0] = getPath( jerb->theJob[0] );
		
		if( jerb->theJob[0] != "" )
		{
			cout << jerb->theJob[0] << endl;
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
			if( execve( jerb->theJob[0].c_str(), argv, envp ) < 0 )
			{	
				cout << "YOU SUCK" << endl;
			}
		}
	}
	else
	{ /* File doesn't exist. */
		cout << "FILE NOT FOUND" << endl;
	}

	wait( NULL );
	/* Reset descriptors. */
	dup2(theStdIn, STDIN_FILENO);
	dup2(theStdOut, STDOUT_FILENO);
}

	
string getPath( string Executable )
{ /* Returns the path of the entered executable in string format. */
	stringstream ss( getenv( "PATH" ) );
	string tokens;
	vector<string> paths;
	while( getline( ss, tokens, ':' ) )
	{
		paths.push_back( tokens );
	}

	for(int i = 0; i < paths.size(); i++)
	{ /* Slap a whack in between the path and executable. */

		paths[ i ] += '/' + Executable;
		

		if( fileExists( paths[ i ] ) )
		{
			return paths[ i ];
		}
	}

	return "";
}

void changeDir( vector<string> command )
{ /* Changes working directory to argument entered. */
    if( command.size() == 1 )
    { /* Switch to home if no directory is entered. */
        if( chdir( getenv( "HOME" ) ) < 0 )
        {
            cout << "ERROR RETURNING TO HOME DIRECTORY" << endl;
        }
    }

    else if( command.size() == 2 )
    { /* Switch to targeted directory, tell user if its not real. */
	if( chdir( command[1].c_str() ) < 0 )
	{
	    cout << "DIR DOES NOT EXIST" << endl;
	}
   }
}

void set( vector<string> command )
{ /* Set the PATH or HOME variables to something the user desires. */
        string newPath;

	if( command.size() == 1 )
	{       /* If there are no arguments, tell user how to use. */
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
		cout << "ID / PID / Doing?\n";
	}

	for(int i = 0; i < theJobs.size(); i ++)

	{ /* Print out each job, specifying what is in the background and what isn't. */
		cout << theJobs[i].id << " " << theJobs[i].pid << " " << theJobs[i].theJob[0] << "\n";
	}
}
 
void kill(vector<string> arg)
{ /* Kill specified process. */
	istringstream is(arg[1]);
	int theInt;
	is >> theInt;
	if(kill(theInt, SIGINT) != 0)
	{ /* If the process cannot be killed. */
		cout << "Process was immortal, could not kill.\n";
	}

	else
	{ /* Process was successfully killed. */
		cout << "Process " << arg[1] << " was killed.\n";
	}
}

bool stdIn(string theFile)
{ /* Copies the file pointer to STDIN_FILENO or notifies user of mistakes. */
	FILE* inFile = fopen(theFile.c_str(), "r");
	if(inFile == NULL)
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
	return true;
}

bool stdOut(string theFile)
{ /* Copies the file pointer to STDOUT_FILENO or notifies user of mistakes. */
		
	FILE* outFile = fopen(theFile.c_str(), "w");
	if(outFile == NULL)
	{ /* Check if writable. */
		cout << "File " << theFile << " does not exist.\n Please check permissions or enter another file.\n";
		return false;
	}

	dup2(fileno(outFile), STDOUT_FILENO);
	fclose(outFile);
	return true;
}

bool fileExists(string theFile)
{ /* Checks to see if a file exists using the access system call, returns 0 if it does,
     returns -1 if it does not, and prints an error message. */
	if(access(theFile.c_str(), F_OK) != 0)
	{ /* Simply check if the file exists. */
		return false;
	}

	return true;
}

job * buildJob( string arg )
{ /* Parses a line of commands, searches for any redirection / background. */
    string iterator;
    stringstream input;
    job * Jerb = new job();
    Jerb->fileIn = "";
    Jerb->fileOut = "";
    Jerb->isBackground = false;

    input << arg;
    while( input >> iterator )
    {
	if( iterator == "<" )
	{ /* Checks for file redirection, prepares it to redirect input. */
		input >> iterator;
		Jerb->fileIn = iterator;
	}

	else if( iterator == ">" )
	{ /* Checks for file redirection, prepares it to redirect output. */
		input >> iterator;
		Jerb->fileOut = iterator;
	}

	else if( iterator == "&" )
	{ /* Checks if it should be run in the background, sets flag. */
		Jerb->isBackground = true;
	}
	
	else
	{ /* Essentially adds the parsed piece as an argument. */
       	 	Jerb->theJob.push_back( iterator );
	}
    }

   return Jerb;
}
       	
