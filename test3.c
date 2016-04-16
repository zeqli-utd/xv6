#include "types.h"
#include "user.h"

int stdout = 1;
#define MAX 1000000

// Starvation Test
// First child process has only 1 ticket
// others are assigned the same amount as their parent
// In each draw, the first process bears little chances
// to win. However, it is still lucky enough winning
// sometimes. So it will eventually finished it's job
// while constantly competing with other processes.
// In this case it competes with 20 normal jobs.
void
lotterytest3(void)
{
  int pid;
  int pmax = 10;     // calculate first 200 prime number (the 200th is 1223 )
  int nmax = 30;
  int i, j, n;
  int nchild = 20;
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

    // give first child process the lowest priority 
    // Only 1 ticket assigned. Default tickets is same as parent
    if(cproc[0] == 0){
      nice(19);     
      for(n = 2 ; n < nmax; n++){
          for (i = 0; i < pmax ; i++){
            if(prime[i] == 0){
              printf(stdout, "PID(%d) (%d tickets) [%d] -> %d\n", getpid(), 41 - (nice(0) - 99), i + 1, n);
              prime[i] = n;
            }
            if(n % prime[i] == 0)
              break;
          }
      }
    }
    else{
      for(i = 0; i < MAX; i++){
        yield();
      }
    }
    if(pid == 0){     
      exit();
    }
  }

  wait();
  for(i = 1; i < nchild; i++){
    kill(cproc[i]);
    wait();
  }  

  printf(stdout, "Starvation Test ok\n\n");
}

int main(void){
	
  lotterytest3();   // Starvation Test
  exit();
}