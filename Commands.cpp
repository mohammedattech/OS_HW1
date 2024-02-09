#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <iomanip>
#include "Commands.h"
#include <algorithm>
#include <signal.h>
#include <fcntl.h>
#include <fstream>
#include <sched.h>
#include <limits.h>

const std::string WHITESPACE = " \n\r\t\f\v";


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

bool _isBackgroundCommand(const char* cmd_line) {
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}
int calculateLength(const char* str)
{
  int len=1;
  while(*(str)!='0')
  {
    len++;
    str++;
  }
  return len;
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
bool findCharacter(const char* cmd_line, char character)
{
  string command=string(cmd_line);
  if(command.find_first_of(character)==string::npos)
  {
    return false;
  }
  return true;
  
}
// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell():m_shellCommands(new JobsList()),m_prompt("smash"),m_lastDirectory(""),m_continueFlag(true),forGroundJob(nullptr)
{
  
}

SmallShell::~SmallShell() 
{
  delete m_shellCommands;
}
const std::string& SmallShell::getPrompt() const
{
  return m_prompt; 
}
void SmallShell::setPrompt(std::string newPrompt)
{
  m_prompt=newPrompt;
}
JobsList* SmallShell::getJobsList()
{
  return m_shellCommands;
}
bool SmallShell::canContinue() const
{
  return m_continueFlag;
}
void SmallShell::EndShell()
{
  m_continueFlag=false;
}
string* SmallShell::getLastDirectory()
{
  return &m_lastDirectory;
}
ExternalCommand* SmallShell::getForgroundJob() const
{
  return forGroundJob;
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {
	// For example:

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
    if (cmd_s.find('|') != string::npos) 
    {
      return new PipeCommand(cmd_line);
    } 
    else if (cmd_s.find('>') != string::npos) 
    {  
      return new RedirectionCommand(cmd_line);
    }
    if (firstWord == "chprompt") 
    {
      return new ChangePromptCommand(cmd_line);
    } 
    else if (firstWord == "showpid" || firstWord == "showpid&") 
    {
      return new ShowPidCommand(cmd_line);
    } 
    else if (firstWord == "pwd" || firstWord == "pwd&") 
    {
      return new GetCurrDirCommand(cmd_line);
    } 
    else if (firstWord == "cd") 
    {
      return new ChangeDirCommand(cmd_line,this->getLastDirectory());
    } 
    else if (firstWord == "jobs" || firstWord == "jobs&")
    {
      return new JobsCommand(cmd_line, m_shellCommands);
    } 
    else if (firstWord == "kill" || firstWord == "kill&") 
    {
      return new KillCommand(cmd_line, m_shellCommands);
    } 
    else if (firstWord == "fg" || firstWord == "fg&") 
    {
      return new ForegroundCommand(cmd_line, m_shellCommands);
    }
    else if (firstWord == "quit" || firstWord == "quit") 
    {
      return new  QuitCommand(cmd_line,m_shellCommands);
    }
    return new ExternalCommand(cmd_line);
}

void SmallShell::executeCommand(const char *cmd_line) {
  // TODO: Add your implementation here
  // for example:
  m_shellCommands->removeFinishedJobs();
  bool shouldBeDeleted=false;
  Command* cmd = CreateCommand(cmd_line);
  if(dynamic_cast<ExternalCommand*>(cmd)==nullptr)
  {
    shouldBeDeleted=true;
  }
  cmd->execute();
  if(shouldBeDeleted)
  {
    delete cmd;
  }
  // Please note that you must fork smash process for some commands (e.g., external commands....)
  //CreateCommand(cmd_line)->execute();
}

Command::Command(const char* cmdline):m_cmdLine(new char[string(cmdline).size()+1]),m_argn(_parseCommandLine(cmdline,nullptr))
{
  m_args=(new char*[m_argn+1]);
  m_args[0]=nullptr;
  _parseCommandLine(cmdline,m_args);
  strcpy(m_cmdLine,cmdline);
}
Command::~Command()
{
  int i=0;
  while(m_args[i])
  {
    free(m_args[i++]);
  }
  delete[] m_args;
  delete[] m_cmdLine;
}

const char* Command::getCommandLine() const
{
  return m_cmdLine;
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
{
  if(_isBackgroundCommand(m_cmdLine))
  {
    int i=0;
    while(m_args[i])
    {
      free(m_args[i++]);
    }
    string duplicate(m_cmdLine);
    int idx=duplicate.find_last_of("&");
    duplicate[idx]=' ';
    m_argn=_parseCommandLine(duplicate.c_str(),m_args);
  }
}
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


ChangeDirCommand::ChangeDirCommand(const char *cmd_line, string* lastPwd) : BuiltInCommand(cmd_line), lastPwd(lastPwd) 
{}

void ChangeDirCommand::ChangeDirTo(const char *path) {
    // file path should be the second argeument
    char CurrWorkingDir[MAX_PATH_LENGTH];
    if(getcwd(CurrWorkingDir, MAX_PATH_LENGTH)==nullptr)
    {
      perror("smash error: getcwd failed");
    }
    else
    {
      if (chdir(path) != 0) 
      {
        perror("smash error: chdir failed");
      } 
      else 
      {
        *lastPwd=CurrWorkingDir;
      }
    }
}

void ChangeDirCommand::execute() {
    if (m_argn == 2) 
    {
        // Check if the argument is "-"
        if (strcmp(m_args[1], "-") == 0) 
        {
          if (*lastPwd == "") 
          {
            printf("smash error: cd: OLDPWD not set");
          } 
          else 
          {
            //change_to_last_directory();
            ChangeDirTo(lastPwd->c_str());
          }
        } 
        else 
        {
          ChangeDirTo(m_args[1]);
        }
    } 
    else 
    {
        printf("smash error: cd: too many arguments");
    }
}

ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList *jobs):  BuiltInCommand(cmd_line),jobs(jobs) {}

void ForegroundCommand::execute() 
{
  int jobId;
  SmallShell& smash = SmallShell::getInstance();
  if(m_argn==1)
  {
    // get the process with the max pid
    ExternalCommand* LastJob= jobs->getLastJob();
    if(LastJob== nullptr)
    {
      // if the vector is empty perror
      perror("smash error: fg: jobs list is empty");
    }
    else
    {
      smash.bringToForeground(LastJob,true);
      //bring to foreground
    }
  }
  else if(m_argn==2)
  {
    // format the arguments to int and if not then perror
    try
    {
      jobId=stoi(m_args[1]);
    }
    catch(const std::exception& e)
    {
      perror("smash error: fg: invalid arguments");
    }
    if(jobs->getJobById(jobId)==nullptr)
    {
      std::ostringstream oss;
      oss << "smash error: fg: job-id " << jobId <<" does not exist";
      perror(oss.str().c_str());
    }
    else
    {
      smash.bringToForeground(jobs->getJobById(jobId),true);
      smash.getJobsList()->removeJobById(jobId);
    }
    // print the process command line
    //TODO bring the process with JOB_Id to the foreground
    //TODO delete all the finshed commands
    }
    else
    {
      perror("smash error: fg: invalid arguments");
    }

}

KillCommand::KillCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line),jobs(jobs) {}

void KillCommand::execute() 
{

  int jobId;
  int signal;
  //SmallShell& smash = SmallShell::getInstance();
  if(m_argn==3)
  {
    //checking the format 
    if(m_args[1][0]!='-')
    {
      perror("smash error: kill: invalid arguments");
    }
    else
    {
      try
      {
        signal=stoi(++m_args[1]);
        jobId=stoi(m_args[2]);
      }
      catch(const std::exception& e)
      {
        perror("smash error: kill: invalid arguments");
      }
      if(jobs->getJobById(jobId)==nullptr)
      {
        perror("smash error: kill: job-id <job-id> does not exist");
      }
      else
      {
        if (kill(jobs->getJobById(jobId)->getPid(),signal)==-1)
        {
          perror("smash error:kill failed");
        }
      }
    }
  }
  else
  {
    perror("smash error: kill: invalid arguments");
  }
}


RedirectionCommand::RedirectionCommand(const char *cmd_line): Command(cmd_line)
{
  string cmd_s(cmd_line);
  append = (cmd_s.find(">>") != string::npos);
  left = cmd_s.substr(0, cmd_s.find('>'));
  right = cmd_s.substr(cmd_s.find_last_of('>') + 1);
  while (left.c_str()[0] == ' ')
    left = left.substr(1);
  while (right.c_str()[right.size() - 1] == ' ')
    right = right.substr(0, right.size() - 1);
}

void RedirectionCommand::execute() 
{
  int fd;
  if (append) 
  {
    fd = open(right.c_str(), O_APPEND | O_WRONLY | O_CREAT , S_IRUSR | S_IWUSR | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH);
  } 
  else 
  {
    fd = open(right.c_str(), O_WRONLY | O_CREAT | O_TRUNC , S_IRUSR | S_IWUSR | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH);
  }
  if (fd == -1) 
  {
    perror("smash error: open failed");
    return;
  }
  int out = dup(1);
  if (out == -1) 
  {
    perror("smash error: dup failed");
    return;
  }
  if (close(1) == -1) 
  {
    perror("smash error: close failed");
    return;
  }
  if (dup2(fd, 1) == -1) 
  {
    perror("smash error: dup2 failed");
    return;
  }
  SmallShell::getInstance().executeCommand(left.c_str());
  if (close(1) == -1) 
  {
    perror("smash error: close failed");
    return;
  }
  if (dup(out) == -1) 
  {
    perror("smash error: dup failed");
    return;
  }
  if (close(out) == -1) 
  {
    perror("smash error: close failed");
    return;
  }
}
PipeCommand::PipeCommand(const char* cmd_line):Command(cmd_line)
{
  string cmd_s(cmd_line);
  inputCommand = cmd_s.substr(0,cmd_s.find('|'));
  outPutCommand = cmd_s.substr(cmd_s.find('|')+1);
  unsigned int pos=outPutCommand.find('&');
  if(pos!=string::npos)
  {
    ToError=true;
    outPutCommand=outPutCommand.substr(pos+1);
  }
}
void PipeCommand::execute()
{
  int fd[2];
  if (pipe(fd) == -1) 
  {
    perror("smash error: pipe failed");
    return;
  }
  pid_t p;
  if ((p = fork()) == 0) 
  {
    if (setpgrp()==-1)
    {
      perror("smash error: setpgrp failed");
    }
    if (ToError) 
    {
      if(dup2(fd[1], 2)==-1)
      {
        perror("smash error: dup2 failed");
        return;
      }
    } 
    else 
    {
      if(dup2(fd[1], 1)==-1)
      {
        perror("smash error: dup2 failed");
        return;
      }
    }
    if(close(fd[0])==-1)
    {
      perror("smash error: close failed");
      return;
    }
    if(close(fd[1])==-1)
    {
      perror("smash error: close failed");
      return;
    }
    SmallShell::getInstance().executeCommand(inputCommand.c_str());
    exit(0);
  }
  pid_t p1;
  if ((p1 = fork()) == 0)
  {
    if (setpgrp()==-1)
    {
      perror("smash error: setpgrp failed");
    }
    if(dup2(fd[0], 0)==-1)
    {
      perror("smash error: dup2 failed");
      return;
    }
    if(close(fd[0])==-1)
    {
      perror("smash error: close failed");
      return;
    }
    if(close(fd[1])==-1)
    {
      perror("smash error: close failed");
      return;
    }
    SmallShell::getInstance().executeCommand(outPutCommand.c_str());
    exit(0);
  }
  if(close(fd[0])==-1)
  {
    perror("smash error: close failed");
    return;
  }
  if(close(fd[1])==-1)
  {
    perror("smash error: close failed");
    return;
  }
  if(waitpid(p, nullptr,WUNTRACED) == -1 || waitpid(p1, nullptr,WUNTRACED) == -1)
  {
    perror("smash error: waitpid failed");
    return;
  }
}

ChmodCommand::ChmodCommand(const char* cmd_line):BuiltInCommand(cmd_line)
{}
void ChmodCommand::execute()
{
  if(m_argn!=3)
  {
    perror("smash error: chmod:invalid arguments");
    return;
  }
  string modestr(m_args[1]);
  modestr.insert(0,1,'0');
  int mode;
  try
  {
    mode=std::stoi(modestr,nullptr,8);
  }
  catch(const std::exception& e)
  {
    perror("smash error: chmod:invalid arguments");
    return;
  }
  if(mode>0777||mode<0)
  {
    perror("smash error: chmod:invalid arguments");
    return;
  }
  if(chmod(m_args[2],mode)==-1)
  {
    perror("smash error: chmod failed");
  }
}


ExternalCommand::ExternalCommand(const char* cmd_line): Command(cmd_line), m_pid(-1), m_backGround(
        _isBackgroundCommand(cmd_line)), m_isComplex(findCharacter(cmd_line, '*') || findCharacter(cmd_line, '?')), m_jobId(-1),m_noAnd(nullptr)
{
  if(m_backGround)
  {
    char* dummy=new char[calculateLength(cmd_line)];
    strcpy(dummy,m_cmdLine);
    _removeBackgroundSign(dummy);
    int i=0;
    while(m_args[i])
    {
      free(m_args[i++]);
    }
    delete[] m_args;
    m_args=new char*[m_argn-1];
    m_argn=_parseCommandLine(dummy,m_args);
    m_noAnd=dummy;
  }
}
ExternalCommand::~ExternalCommand()
{
  if(m_noAnd)
  {
    delete[] m_noAnd;
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
void ExternalCommand::assignJobId(int id)
{
  m_jobId =id;
}
void ExternalCommand::setPid(pid_t pid)
{
  m_pid=pid;
}
pid_t ExternalCommand::getPid()const
{
  return m_pid;
}
int ExternalCommand::getJobId() const
{
  return m_jobId;
}
void ExternalCommand::execute()
{
  char* Command=m_cmdLine;
  if(m_backGround)
  {
    Command=m_noAnd;
  }
  char* cmd="./bin/bash";
  char* const argv[]={"./bin/bash","-c",Command};
  pid_t sonPid=fork();
  if(sonPid==-1)
  {
    perror("smash error: fork failed");
  }
  else
  {
    if(sonPid==0)
    {
      if(setpgrp()==-1)
      {
        perror("smash error: setpgrp failed");
      }
      if(m_isComplex)
      {
        execvp(cmd,argv);
      }
      else
      {
        execvp(m_args[0],m_args);
      }
      perror("smash error: execvp failed");
      exit(0);
    }
    else
    {
      m_pid=sonPid;
      SmallShell& shell=SmallShell::getInstance();
      if(m_backGround)
      {
        shell.getJobsList()->addJob(this);
      }
      else
      {
        shell.bringToForeground(this,false);
      }
    }
  }
}
JobEntry::JobEntry(int jobId):m_jobId(jobId)
{}
int JobEntry::getJobId() const
{
  return m_jobId;
}
JobsList::~JobsList()
{
  for(ExternalCommand* cmd:m_jobs)
  {
    delete cmd;
  }
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
    jobId=m_jobs.back()->getJobId()+1;
  }
  cmd->assignJobId(jobId);
  m_jobs.push_back(cmd);
}
void JobsList::printJobsList()
{
  for (const ExternalCommand* job: m_jobs)
  {
    std::cout << "[" << job->getJobId() << "]" << " " << job->getCommandLine() << std::endl;
  }
}
void JobsList::killAllJobs()
{
  std::cout <<"smash: sending SIGKILL signal to " << m_jobs.size() << " jobs:" <<std::endl;
  for(const ExternalCommand* job: m_jobs)
  {
    std::cout << job->getPid() << ": " << job->getCommandLine() << std::endl;
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
  vector<ExternalCommand*>::iterator newEnd=std::remove_if(m_jobs.begin(),m_jobs.end(),JobFinished);
  for(vector<ExternalCommand*>::iterator i=newEnd;i<m_jobs.end();i++)
  {
    delete (*i);
  }
  m_jobs.erase(newEnd,m_jobs.end());
}
ExternalCommand* JobsList::getJobById(int jobId)
{
  ExternalCommand* cmd=nullptr;
  for(ExternalCommand* job:m_jobs)
  {
    if(job->getJobId()==jobId)
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
    if((*i)->getJobId()==jobId)
    {
      m_jobs.erase(i);
      break; 
    }
  }
}

ExternalCommand *JobsList::getLastJob() 
{
  ExternalCommand* jobToReturen = nullptr;
  if (!m_jobs.empty()) 
  {
    jobToReturen = m_jobs.back();
  }
  return jobToReturen;
}

void SmallShell::bringToForeground(ExternalCommand* cmd,bool fromFg)
{
  m_shellCommands->removeJobById(cmd->getJobId());
  forGroundJob=cmd;
  if(fromFg)
  {
    std::cout << cmd->getCommandLine() << " " << cmd->getPid() <<std::endl;
  }
  if(waitpid(cmd->getPid(),nullptr,0)==-1)
  {
    perror("smash error: waitpid failed");
  }
  delete forGroundJob;
  forGroundJob=nullptr;
}




