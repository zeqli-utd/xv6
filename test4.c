#include "types.h"
#include "user.h"

int stdout = 1;
#define MAX 1000000

// The priority test, the first job will get the highest
// priority, as it will complete first.
void
policytest(void)
{
  int pid;
  int pmax = 200;
  int nmax = 1230;      // calculate first 200 prime number (the 200th is 1223 )
  int i, j, n;
  int nchild = 3;
  int cproc[nchild];
  int prime[pmax];
  for(i=0;i<pmax;i++)
    prime[i] = 0;
  for(i=0;i<nchild;i++)
    cproc[i] = 0;

  // Set to Round-Robin scheduling
  if(salgo(0) == 0)
    printf(stdout, "Switch scheduling strategy to Round-Robin.\n");
  else
    printf(stdout, "Switch Round-Robin failed.\n");

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
            printf(stdout, "PID(%d) (%d tickets) [%d] -> %d\n", getpid(), 41 - (nice(0) - 99), i + 1, n);
            prime[i] = n;
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
  
  // Set back to lottery ticket
  if(salgo(1) == 1)
    printf(stdout, "Switch scheduling strategy to Loterry scheduling.\n");
  else
    printf(stdout, "Switch Loterry scheduling failed.\n");

  printf(stdout, "Switch strategy test ok\n\n");
}

int main(void){
  policytest();	  	// Switching scheduling policy test.
  exit();
}