// jobs.cpp
struct job
{ // Variables to hold job info.
	std::string command;
	int id;
	pid_t pid;
}

void jobs()
{ // Print all jobs.
	for(int i = 0; i < theJobs.size(); i ++)
		cout << theJobs[i].id << " " << theJobs[i].pid << " " << theJobs[i].command << "\n";
}
