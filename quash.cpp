#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <vector>

using namespace std;

void changeDir( vector<string> test );
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
       	
