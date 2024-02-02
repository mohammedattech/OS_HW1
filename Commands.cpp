#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

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
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
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
Command::Command(const char* cmdline):m_cmdLine(cmdline)
{
  m_args[0]=nullptr;
  m_argn=_parseCommandLine(cmdline,m_args);
}
Command::~Command()
{
  int i=0;
  while(m_args[i])
  {
    free(m_args[i++]);
  }
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
    perror("smash Error:getpid failed");
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
  long pathMax=pathconf(path,_PC_PATH_MAX);
  if(pathMax==-1)
  {
    perror("smash error: pathconf failed");
    pathMax=MAX_PATH_LENGTH;
  }
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
