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

int jobId;

struct job
{ /* Variables to hold job info. */
	bool isBackground;
	vector<string> theJob;
	string fileIn;
	string fileOut;
	int id;
	pid_t pid;
};

void sigHandler(int signal);
void kill(vector<string> arg);
void jobs();
void changeDir( vector<string> test );
void set( vector<string> command );
vector<job *> buildJob( string arg );
bool fileExists( string theFile );
string getPath( string );
bool stdIn(string theFile);
bool stdOut(string theFile);
void executeJob( vector<job *> jerb, char ** );

vector<job *> theJobs; /* data structure to hold background processes */


int main( int argc, char **argv, char **envp )
{ /* Quash, its Quite an Awesome SHell, isn't it? */

	/* set up monitoring of background processes */
	struct sigaction sig;
	
	memset( &sig, 0, sizeof( sig ) );
	sig.sa_handler = sigHandler;

	if( sigaction( SIGCHLD, &sig, NULL ) == -1 )
	{
		cerr << "Failure in signal monitoring." << endl;
		exit( 1 );
	} 

   vector<job *> jerb;
   string commandLine;
   cout << "quash~$ ";
   while( getline( cin, commandLine ) )
   { /* Read prompt. */
	cout << "quash~$ ";
       jerb = buildJob( commandLine );

       if( jerb[0]->theJob[ 0 ] == "quit" || jerb[0]->theJob[ 0 ] == "exit" )
       { /* Exit Quash. */
            exit( 0 );
       }
       else if( jerb[0]->theJob[ 0 ] == "cd" )
       { /* Change Directory. */
	    changeDir( jerb[0]->theJob );
       }
       else if( jerb[0]->theJob[ 0 ] == "set" )
       { /* Set PATH or HOME. */
            set( jerb[0]->theJob );
       }
       else if( jerb[0]->theJob[ 0 ] == "jobs" )
       { /* List jobs. */
	     jobs();
       }
       else if( jerb[0]->theJob[ 0 ] == "kill" )
       { /* Kill job. */
            kill ( jerb[0]->theJob );
       }
       else
       { /* Execute job. */
        	executeJob( jerb, envp );
       }
   }
  
}

void executeJob( vector<job *> jerb, char ** envp )
{ /* Executes an executable, with or without arguments, while passing down
     the environment to all child processes. */
	/* Save descriptors. */
	int theStdIn = dup(STDIN_FILENO);
	int theStdOut = dup(STDOUT_FILENO);

	/* init pipe to support single pipe */
	int pipes[2];
       if( pipe(pipes) < 0 )
	{
		cout << " error with piping" << endl;
	}
	for( int i = 0; i < jerb.size(); i++ )
	{
	bool exists;
	pid_t process;

				
	if(jerb[i]->fileIn != "")
	{ /* Redirect input. */
		stdIn(jerb[i]->fileIn);
	}

	if(jerb[i]->fileOut != "")
	{ /* Redirect output. */
		stdOut(jerb[i]->fileOut);
	}

	/* translate arguements from a vector to a format execve can understand */
	char ** argv = new char*[ jerb[i]->theJob.size() + 1 ];
	
	for(  int j = 0; j < jerb[i]->theJob.size(); j++) 
	{
		argv[ j ] = new char[jerb[i]->theJob[j].length() + 1];
		strcpy(argv[j], jerb[i]->theJob[j].c_str());
	}

	argv[ jerb[i]->theJob.size() ] = NULL;  

	if( jerb[i]->theJob[0][0] == '/' )
	{ /* Path is absolute. */
		exists = fileExists( jerb[i]->theJob[0] );
	}

	else if( !strncmp( "./", jerb[i]->theJob[0].c_str(), 2 ) )
	{ /* Path is relative. */
		jerb[i]->theJob[0].erase( 0, 2 );
		exists = fileExists( jerb[i]->theJob[0] );
	}

	else
	{ /* Path is part of PATH. */

		jerb[i]->theJob[0] = getPath( jerb[i]->theJob[0] );
		
		if( jerb[i]->theJob[0] != "" )
		{ /* Job is not empty. */
			exists = true;
		}

		else
		{
			exists = false;
		}
	}

	if( exists )
	{ /* If the file exists. */
;
		pid_t pid = fork();

		
		if( pid	< 0 )
		{ /* Child forked badly. */
			cout << "Problem forking child" << endl;
		}
		else if( pid == 0 )
		{ /* Child forked. */
			if( jerb[i]->isBackground )
			{ /* Check for background flag. */
				setpgid( 0, 0 );
			}
			/* Support for single pipe */
			if( i == 0 && jerb.size() == 2 )
			{
				close( pipes[0] );
				dup2( pipes[1], STDOUT_FILENO ); /* set write end */
				close(pipes[1]);
			}
			else if( i == 1 && jerb.size() == 2 )
			{
				close( pipes[1] );
				dup2( pipes[0], STDIN_FILENO ); /* set read end if second process */
				close(pipes[0]);

			}

			/* by using execve we can pass enviroment to child process */
			if( execve( jerb[i]->theJob[0].c_str(), argv, envp ) < 0 )
			{ /* Notify if execution fails. */
				cout << "Error in execution of job" << endl;
			}
		}
		else
		{

			if( jerb[i]->isBackground == true )
			{/* Check for background flag. */
				close(pipes[1]);
				close(pipes[2]);
				jerb[i]->id = jobId++;
				jerb[i]->pid = pid;
				cout << "[ " << jerb[i]->id << " ] " << jerb[i]->pid << " running in the background." << endl;
				theJobs.push_back( jerb[i] );
			}
			else
			{ /* Not background, wait for it to finish. */
			close(pipes[1]);
			close(pipes[2]);
				wait( NULL );
			}

		}

	}

	else
	{ /* File doesn't exist. */
		cout << "FILE NOT FOUND" << endl;
	}

	}
	close(pipes[0]);
	close(pipes[1]);
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
		cout << theJobs[i]->id << " " << theJobs[i]->pid << " " << theJobs[i]->theJob[0] << "\n";
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

vector<job *> buildJob( string arg )
{ /* Parses a line of commands, searches for any redirection / background. */
    string iterator;
    stringstream input;
    vector<job *> Jerb;
    job * job1;
    int i = 0;

    job1 = new job;

    job1->fileIn = "";
    job1->fileOut = "";
    job1->isBackground = false;



    Jerb.push_back( job1 );
 
 
    input << arg;
    while( input >> iterator )
    {
	if( iterator == "<" )
	{ /* Checks for file redirection, prepares it to redirect input. */
		input >> iterator;
		Jerb[i]->fileIn = iterator;
	}

	else if( iterator == ">" )
	{ /* Checks for file redirection, prepares it to redirect output. */
		input >> iterator;
		Jerb[i]->fileOut = iterator;
	}

	else if( iterator == "&" )
	{ /* Checks if it should be run in the background, sets flag. */
		Jerb[i]->isBackground = true;
	}

	/* detecting piping */
	else if( iterator == "|" )
	{
            i++;
    	    job * job2;
	    job2 = new job;

	    job2->fileIn = "";
	    job2->fileOut = "";
	    job2->isBackground = false;
	   Jerb.push_back( job2 );
	}
	
	else
	{ /* Essentially adds the parsed piece as an argument. */
       	 	Jerb[i]->theJob.push_back( iterator );
	}
    }

   return Jerb;
}

void sigHandler(int signal) 
{
	pid_t pid;
	/* if a background process finishes remove from background job datastructure and report the fact that it finished */
	while( (pid = waitpid( -1, NULL, WNOHANG)) > 0 )
	 {

		for( int i = 0; i < theJobs.size(); i++ )
		{
			cout << pid << endl;
			if( theJobs[i]->pid == pid )
			{

				cout << endl << "[ " << theJobs[i]->id << " ] " << theJobs[i]->pid << " finished " << theJobs[i]->theJob[0] << endl;

				theJobs.erase(theJobs.begin() + i);

				break;
			}
		}
	}
}
       	
