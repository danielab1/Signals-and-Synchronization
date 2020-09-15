#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid, signum;

  if(argint(0, &pid) < 0 || argint(1, &signum) < 0 || signum<0 || 31<signum)
    return -1;
  return kill(pid, signum);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_sigprocmask(void){
  int newmask;
  if(argint(0, &newmask)<0){
    return -1;
  }
  uint oldmask = myproc()->signal_mask;
  myproc()->signal_mask = (uint)newmask;
  return oldmask;
}

int
sys_sigaction(void){
  int signum;
  struct sigaction* act = 0;
  struct sigaction* oldact = 0;
  if(argint(0, &signum)<0 || signum<0 || 31<signum || signum==SIGSTOP || signum==SIGKILL){
    return -1;
  }
  if(argptr(1, (void*)&act, sizeof(*act))<0 || argptr(2, (void*)&oldact, sizeof(*act))<0){
    return -1;
  }
  struct sigaction* curr_handler = myproc()->sig_handlers_ptrs[signum];
  void* oldsa = curr_handler->sa_handler;
  uint oldmask = curr_handler->sigmask;
  
  if(oldact != 0){
    oldact->sa_handler = oldsa;
    oldact->sigmask = oldmask;
  }

  memmove(curr_handler, act, sizeof(struct sigaction));
  return 0;
}

int
sys_sigret(void){
  memmove(myproc()->tf, myproc()->user_tf, sizeof(struct trapframe));
  myproc()->signal_mask = myproc()->signal_mask_backup;
  myproc()->nested = 0;
  myproc()->tf->esp += sizeof (struct trapframe);
  return 0;
}