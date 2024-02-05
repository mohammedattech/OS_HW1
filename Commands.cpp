#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;
static char WHITESPACE=' ';
#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) 
  {
    if(args)
    {
      args[i] = (char*)malloc(s.length()+1);
      memset(args[i], 0, s.length()+1);
      strcpy(args[i], s.c_str());
      args[++i] = NULL;
    }
    else
    {
      i++;
    }
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundComamnd(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char* cmd_line) 
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}
bool findCharachter(const char* cmd_line,char character)
{
  string command=string(cmd_line);
  if(command.find_first_of(character)==string::npos)
  {
    return false;
  }
  return true;
  
}
// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
// TODO: add your implementation
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	// For example:
/*
  string cmd_s = _trim(string(cmd_line));
  string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

  if (firstWord.compare("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if (firstWord.compare("showpid") == 0) {
    return new ShowPidCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  // Command* cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
  char** args;
  int argn=_parseCommandLine(cmd_line,args);
  //CreateCommand(cmd_line)->execute();
}
Command::Command(const char* cmdline):m_cmdLine(cmdline),m_argn(_parseCommandLine(cmdline,nullptr))
{
  m_args=(new char*[m_argn+1]);
  m_args[0]=nullptr;
  _parseCommandLine(cmdline,m_args);
}
Command::~Command()
{
  int i=0;
  while(m_args[i])
  {
    free(m_args[i++]);
  }
  delete[] m_args;
}
const char* Command::getCommandLine() const
{
  return m_cmdLine.c_str();
}
char** Command::getArguments() const
{
  return m_args;
}
int Command::getNumberOfArguments() const
{
  return m_argn;
}
BuiltInCommand::BuiltInCommand(const char* cmdline):Command(cmdline)
{}
ChangePromptCommand::ChangePromptCommand(const char* cmd_line):BuiltInCommand(cmd_line)
{}
void ChangePromptCommand::execute()
{
  SmallShell& smash = SmallShell::getInstance();
  if(m_argn==1)
    {
      smash.setPrompt("smash");
    }
    else
    {
      smash.setPrompt(m_args[1]); 
    }

}
ShowPidCommand::ShowPidCommand(const char* cmd_line): BuiltInCommand(cmd_line)
{}
void ShowPidCommand::execute()
{
  pid_t smashPid=getpid();
  if(smashPid==-1)
  {
    perror("smash error:getpid failed");
  }
  else
  {
    cout << "smash pid is " << smashPid <<endl;
  }
}
GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line):BuiltInCommand(cmd_line)
{}
void GetCurrDirCommand::execute()
{
  const char* path=".";
  int pathMax=MAX_PATH_LENGTH;
  char* buffer = new char[pathMax];
  if (getcwd(buffer, pathMax) != nullptr) 
  {
    std::cout << buffer << std::endl;
  } 
  else 
  {
    perror("smash error: getcwd failed");
  }
  delete[] buffer;
}
JobsCommand::JobsCommand(const char* cmd_line,JobsList* jobs):BuiltInCommand(cmd_line),m_list(jobs)
{}
void JobsCommand::execute()
{
  m_list->printJobsList();
}
QuitCommand::QuitCommand(const char* cmd_line,JobsList* jobs):BuiltInCommand(cmd_line),m_list(jobs)
{}
void QuitCommand::execute()
{
  if((m_argn==2)&&(strcmp(m_args[1],"kill")==0))
  {
    m_list->killAllJobs();
  }
  SmallShell::getInstance().EndShell();
}

ExternalCommand::ExternalCommand(const char* cmd_line):Command(cmd_line),m_pid(-1),m_backGround(_isBackgroundComamnd(cmd_line)),m_isComplex(findCharachter(cmd_line,'*')||findCharachter(cmd_line,'?')),m_listEntry(nullptr)
{}
ExternalCommand::~ExternalCommand()
{
  if(m_listEntry)
  {
    delete m_listEntry;
  }
}
bool ExternalCommand::complex() const
{
  return m_isComplex;
}
bool ExternalCommand::backGround() const
{
  return m_backGround;
}
void ExternalCommand::addEntry(JobEntry* entry)
{
  m_listEntry = entry; 
}
JobEntry* ExternalCommand::getJobEntry() const
{
  return m_listEntry;
}
void ExternalCommand::setPid(pid_t pid)
{
  m_pid=pid;
}
pid_t ExternalCommand::getPid()const
{
  return m_pid;
}
void ExternalCommand::execute()
{
  
}
JobEntry::JobEntry(int jobId):m_jobId(jobId)
{}
int JobEntry::getJobId() const
{
  return m_jobId;
}
void JobsList::addJob(ExternalCommand* cmd)
{
  int jobId;
  if(m_jobs.size()==0)
  {
    jobId = 1;
  }
  else
  {
    jobId=m_jobs.back()->getJobEntry()->getJobId()+1;
  }
  cmd->addEntry(new JobEntry(jobId));
  m_jobs.push_back(cmd);
}
void JobsList::printJobsList()
{
  for (const ExternalCommand* job: m_jobs)
  {
    std::cout << "[" << job->getJobEntry()->getJobId() << "]" << " " << job->getCommandLine() << std::endl;
  }
}
void JobsList::killAllJobs()
{
  for(const ExternalCommand* job: m_jobs)
  {
    if(kill(job->getPid(),SIGKILL)==-1)
    {
      perror("smash error:kill failed");
    }
  }
}
bool JobFinished(ExternalCommand* cmd)
{
  pid_t status=waitpid(cmd->getPid(),nullptr,WNOHANG);
  if(status==-1)
  {
    perror("smash error:waitpid failed");
  }
  if(status>0)
  {
    return true;
  }
  return false;
}
void JobsList::removeFinishedJobs()
{
  m_jobs.erase(std::remove_if(m_jobs.begin(),m_jobs.end(),JobFinished),m_jobs.end());
}
ExternalCommand* JobsList::getJobById(int jobId)
{
  ExternalCommand* cmd=nullptr;
  for(ExternalCommand* job:m_jobs)
  {
    if(job->getJobEntry()->getJobId()==jobId)
    {
      cmd=job;
    }
  }
  return cmd;
}
void JobsList::removeJobById(int jobId)
{
  for(std::vector<ExternalCommand*>::iterator i=m_jobs.begin();i!=m_jobs.end();i++)
  {
    if((*i)->getJobEntry()->getJobId()==jobId)
    {
      m_jobs.erase(i);
      break; 
    }
  }
}