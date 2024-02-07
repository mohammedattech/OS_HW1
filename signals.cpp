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
        perror("smash error:kill failed");
      }
      else
      {
        std::cout << "smash: procces " << forgroundCommand->getPid() << " was killed";
      }
    }
  }
}

void alarmHandler(int sig_num) {
  // TODO: Add your implementation
}

