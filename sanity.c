#include "types.h"
#include "stat.h"
#include "user.h"

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200

void infinite_loop_sig_handler(int signum){
    printf(1, "handler for signum: %d\n", signum);
    kill(getpid(), 6);
}

void print_some_else(int signum){
    printf(1, "Executing handler for: %d\n", signum);
}

void sigstop_sigcount(){
  int cpid = 0;
  if((cpid = fork()) == 0){
    for(;;){
        printf(1,"forever loop\n");
    }
  }
  kill(cpid,17);
  printf(1,"the child need to be stoped\n");
  sleep(10);
  kill(cpid,9); //terminate the child
  printf(1, "the child need to die\n");
  wait();
}

void sigstop_andkill(){
  int cpid = 0;
  if((cpid = fork()) == 0){
    sleep(5);
    for(;;){
        printf(1,"forever loop\n");
    }
  }
  kill(cpid,17);
  printf(1,"the child need to be stoped\n");
  sleep(10);
  kill(cpid,19); //now the child will print
  sleep(10);
  kill(cpid,9); //terminate the child
  printf(1, "the child need to die\n");
  wait();
}

int
main(int argc, char *argv[])
{   
  uint newmask =  (1<<10) | (1<<18), 
  mask;
  int cpid;
  struct sigaction handler = {
      infinite_loop_sig_handler,
      newmask
  };
  struct sigaction old_handler;

  //--------- 2.2 MASKS -------------
  printf(1,"new mask is: 0x%p\n", newmask);
  mask = sigprocmask(newmask);
  printf(1,"mask before replacement: 0x%p\n", mask);
  newmask = sigprocmask(newmask);
  printf(1,"mask after replacement: 0x%p\n", newmask);

  // -------- 2.1.4 registering signal handlers ---------
  // (1) making sure the sig handler can not be changes after it was set
  sigaction(6, &handler, 0); //handler for signum 6 is now handler
  sigaction(6, &handler, &old_handler); //old_handler supposed to hold handler fields' values
  handler.sa_handler = print_some_else; //change the handler from user space
  sigaction(6, &handler, &old_handler); 
  //if old_handler.sa is the same as print_some_else, then it was changed from the outside
  printf(1, "old handler action: 0x%p expected addr: 0x%p\n", old_handler.sa_handler, infinite_loop_sig_handler);

  //------ 2.4 -----------
  // (1) ignoring sigkill
  if(fork() == 0){
    mask = sigprocmask(1<<9); // 1 in the 9th bit means ignoring the bit
    kill(getpid(), 9);
    printf(1, "ERROR: it ignores kill, test 2.4 (1)\n");
  }
  
  // (2) SIG_DFL kills the process
  if(fork() == 0){
    kill(getpid(), 5);
    printf(1, "ERROR: Default handler doesnt kill the process, test 2.4 (2)\n");
  } 

  // (3) SIGSTOP-SIGCONT test
  // sholud print: parent1, child, parent2
  if((cpid = fork()) == 0){
    sleep(20);
    printf(1,"Child\n");
    exit();
  } else {
    kill(cpid,17);
    sleep(100);
    printf(1,"Parent1\n");
    kill(cpid, 19);
    sleep(100);
    printf(1,"Parent2\n");
  }
  
  // (4) SIG_IGN is really ignoring the function
  handler.sa_handler = (void*)1; //1=SIG_IGN
  sigaction(1, &handler, 0);
  kill(getpid(), 1);

  // (5) no NESTED user signal handlers
  handler.sa_handler = infinite_loop_sig_handler;
  sigaction(6, &handler, 0);
  kill(getpid(), 6);

  // (6) making sure the mask was set back to initial mask
  // if the forked process continues instead of exiting that means
  // that the flag for sig num 10 is 0 instead of 1
  if(fork() == 0){
    kill(getpid(), 10);
    exit();
  }
  
  if((cpid = fork()) == 0){
    for(;;)
    ;
  } else {
    kill(cpid, 9);
  }
  wait();
  wait();
  wait();
  wait();
  wait();
  sigstop_sigcount();
  sigstop_andkill();
  exit();
}

