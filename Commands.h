#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <sys/types.h>
#include "signals.h"
#include <algorithm>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class JobsList;
class JobEntry;
class Command {
 protected:
  std::string m_cmdLine;
  int m_argn;
  char** m_args;
 public:
  Command(const char* cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  const char* getCommandLine() const;
  char** getArguments() const;
  int getNumberOfArguments() const;
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line);
  virtual ~BuiltInCommand()= default;
  virtual void execute()=0;
};

class ExternalCommand : public Command 
{
  pid_t m_pid;
  bool m_backGround;
  bool m_isComplex;
  JobEntry* m_listEntry;
 public:
  ExternalCommand(const char* cmd_line);
  virtual ~ExternalCommand();
  void execute() override;
  bool backGround() const;
  bool complex() const;
  void addEntry(JobEntry* entry);
  void setPid(pid_t pid);
  pid_t getPid() const;
  JobEntry* getJobEntry() const;
};

class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 // TODO: Add your data members
 public:
  explicit RedirectionCommand(const char* cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  //void prepare() override;
  //void cleanup() override;
};
class ChangePromptCommand : public BuiltInCommand
{
public:
  ChangePromptCommand(const char* cmd_line);
  virtual ~ChangePromptCommand()=default;
  void execute() override;
};


class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
  ChangeDirCommand(const char* cmd_line, char** plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand 
{
 public:
  static const int MAX_PATH_LENGTH=80;
  GetCurrDirCommand(const char* cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line);
  virtual ~ShowPidCommand() = default;
  void execute() override;
};


class QuitCommand : public BuiltInCommand {
  private:
   JobsList* m_list;
  public:
   QuitCommand(const char* cmd_line, JobsList* jobs);
   virtual ~QuitCommand()=default;
   void execute() override;
};


class JobEntry
{
private:
    int m_jobId;
public:
    JobEntry(int jobId);
    ~JobEntry()=default;
    int getJobId() const;
};

class JobsList {
 private:
  std::vector<ExternalCommand*> m_jobs;//can be changed to external command instead of Command but wanted to keep it for know just in case we need it
 // TODO: Add your data members
 public:
  JobsList()=default;
  ~JobsList()=default;
  void addJob(ExternalCommand* cmd);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  ExternalCommand * getJobById(int jobId);
  void removeJobById(int jobId);
  ExternalCommand* getLastJob();
  //JobEntry *getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
}; 

class JobsCommand : public BuiltInCommand {
 private:
  JobsList* m_list;
 public:
  JobsCommand(const char* cmd_line, JobsList* jobs);
  virtual ~JobsCommand()=default;
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  KillCommand(const char* cmd_line, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 // TODO: Add your data members
 public:
  ForegroundCommand(const char* cmd_line, JobsList* jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

class ChmodCommand : public BuiltInCommand {
 public:
  ChmodCommand(const char* cmd_line);
  virtual ~ChmodCommand() {}
  void execute() override;
};


class SmallShell {
 private:
  JobsList* m_shellCommands;
  std::string m_prompt;
  std::string m_lastDirectory;
  bool m_continueFlag;
  ExternalCommand* forGroundJob;
  SmallShell();
 public:
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  const std::string& getPrompt() const;
  const std::string& getLastCommand() const;
  void setPrompt(std::string newPrompt);//maybe should be changes to const&
  JobsList* getJobsList();
  bool canContinue() const;
  void EndShell();
  // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_
