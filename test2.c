#include "types.h"
#include "user.h"

int stdout = 1;
#define MAX 1000000

// Dynamic Test
// The first child process obtains highest priority,
// while at 100th prime number, it drops dramatically 
// to 1 ticket. As in the standard output, the first
// job progress faster than others, but eventually 
// it lose the game.
void
lotterytest2(void)
{
  int pid;
  int pmax = 200;
  int nmax = 1230;  // calculate first 200 prime number (the 200th is 1223 )
  int i, j, n;
  int nchild = 3;
  int cproc[nchild];
  int prime[pmax];
  for(i=0;i<pmax;i++)
    prime[i] = 0;
  for(i=0;i<nchild;i++)
    cproc[i] = 0;

  for(j = 0; j < nchild; j++){
    pid = fork();
    if(pid > 0){
      cproc[j] = pid;
      continue;
    }
    if(pid < 0){
      printf(stdout, "fork failed\n");
      exit();
    }

    // give first child process the highest priority 
    // 40 tickets assigned. Default tickets is same as parent
    if(cproc[0] == 0)
      nice(-20);     

    // Calculate Prime Number
    for(n = 2 ; n < nmax; n++){
        for (i = 0; i < pmax ; i++){
          if(prime[i] == 0){
            prime[i] = n;
            printf(stdout, "PID(%d) (%d tickets) [%d] -> %d\n", getpid(), 41 - (nice(0) - 99), i + 1, n);
            // When the first process has found the 100th prime number
            // Slow it down with only 1 ticket
            if(cproc[0] == 0 && i == pmax/2)  
              nice(19);
          }
          if(n % prime[i] == 0)
            break;
        }
    }
    
    if(pid == 0){     
      exit();
    }
  }
  while(nchild--){
    wait();
  }
  
  printf(stdout, "Dynamic Test ok\n\n");
}

int main(void){
  lotterytest2();   // Dynamic test
  exit();
}