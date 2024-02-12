#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlCHandler(int sig_num) 
{
  if(sig_num==SIGINT)
  {
    std::cout << "smash: got ctrl-C" <<std::endl;
    SmallShell& shell=SmallShell::getInstance();
    ExternalCommand* forgroundCommand=shell.getForgroundJob();
    if(forgroundCommand)
    {
      if(kill(forgroundCommand->getPid(),SIGKILL)==-1)
      {
        std::cerr<<"smash error: kill failed"<<std::endl;
      }
      else
      {
        std::cout << "smash: process " << forgroundCommand->getPid() << " was killed" << std::endl;
      }
    }
  }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

