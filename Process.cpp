#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include "Process.hpp"

Process::Process(const std::vector<char*>& args, bool verbose) : 
    verbose(verbose), 
    m_name(args[0]),
    m_pid((pid_t)NULL),
    m_writepipe {-1,-1},
    m_readpipe {-1,-1},
    m_pwrite((FILE*)NULL),
    m_pread((FILE*)NULL)
{
    pipe(m_writepipe);
    pipe(m_readpipe);
    
    if ((m_pid = fork()) < 0)
    {
		perror("Process fork");
		throw std::string("Process fork");
    } else if ( m_pid == 0 ) {
		/* child process */
		close(m_readpipe[0]);
		dup2(m_readpipe[1], 1);
		close(m_writepipe[1]);
		dup2(m_writepipe[0],0);
		
		execvp(args[0], const_cast<char**>(&args[0]));
		perror("Process execvp");
		throw std::string("Process execvp");
    } else { 
		/* parent process */
		close(m_readpipe[1]);
		dup2(m_readpipe[0], 0);
		close(m_writepipe[0]);
		dup2(m_writepipe[1],1);
		

		m_pwrite = fdopen(m_writepipe[1], "a");
		
		if (verbose)
			std::cerr << "Process " << m_name << ": forked PID " << m_pid << std::endl;
    }
};

Process::~Process()
{
    if (verbose)
		std::cerr << "Process " << m_name << ": Entering ~Process()" << std::endl;
    
    kill(m_pid, SIGTERM);
    int status;
    pid_t pid = waitpid(m_pid, &status, 0);
    if (pid < 0)
		perror("~Process waitpid");

    if (verbose)
		std::cerr << "Process " << m_name << ": Leaving ~Process()" << std::endl;
};

void Process::write(const std::string& message) {
//	m_pwrite << message;
}

std::string Process::read() {
	std::string message;
//	size_t num_bytes = 0;
	m_pread = fdopen(m_readpipe[0], "r");
	m_pread >> message;
	fclose(m_pread);
	return message;
}
