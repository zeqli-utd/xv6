#include "types.h"
#include "user.h"

int stdout = 1;
char buf[8192];
#define MAX 1000000

void
wolfietest(void)
{
  // char *p = (char*)malloc(sizeof(buf));
  memset(&buf, 0, sizeof(buf));
  uint sz = wolfie(buf, sizeof(buf));
  if(sz < 0){
  	printf(stdout, "copy wolfie ascii failed.\n");
  	exit();
  }
  printf(stdout, "%d bytes are copied\n\n%s\n", sz, buf);
  printf(stdout, "wolfie test ok.\n");
}

void
nicevaltest(void)
{
  int unice;
  int pid;
  
  if(nice(3) < 0){     // The niceval of child proc is set to be 139
      printf(stdout, "Set child nice val failed.\n");
      exit();
  }
  printf(stdout, "parent (%d) : nice(0) = %d\n",getpid(), nice(0)); 
  pid = fork();
  if(pid < 0){
    printf(stdout, "fork failed\n");
    exit();
  }
  
  
  if(pid == 0){     
    printf(stdout, "child (%d) : nice(0) = %d\n",getpid(), nice(0));
    if(nice(19) < 0){     // The niceval of child proc is set to be 139
      printf(stdout, "Set child nice val failed.\n");
      exit();
    }
    printf(stdout, "child (%d) : nice(19) succeed.\n", getpid());  

    if(nice(0) != 139){ 
      printf(stdout, "Get child nice val failed.\n");
      exit();
    }
    printf(stdout, "child (%d) : nice(0) = %d\n",getpid(), nice(0));
    exit();
  }

  wait();
	
  if(nice(-20) < 0){     // Nice val was set to 100
    printf(stdout, "Set nice val failed.\n");
    exit();
  }
  printf(stdout, "parent (%d) : nice(-20) succeed \n", getpid()); 
  unice = nice(0);
  if(unice != 100){ 
    printf(stdout, "Get nice val failed. (%d)\n", unice);
    exit();
  }
  printf(stdout, "parent (%d) : nice(0) = %d\n",getpid(), nice(0));
  
  if(nice(-21) == 0){
  	printf(stdout, "error: niceval less than -20 %d\n", nice(0));
  	exit();
  }
  if(nice(20) == 0){
  	printf(stdout, "error: niceval greater than 19 %d\n", nice(0));
  	exit();
  }
  nice(1);
  printf(stdout, "niceval test ok\n");
}

void
prng_test(void){
  prngtest();
  printf(stdout, "prng test ok\n\n");
  sleep(3);
}


// The priority test, the first job will get the highest
// priority, as it will complete first.
void
lotterytest1(void)
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
  
  printf(stdout, "Priority Test ok\n\n");
}


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


// Starvation Test
// First child process has only 1 ticket
// others are assigned the same amount as their parent
// In each draw, the first process bears little chances
// to win. However, it is still lucky enough winning
// sometimes. So it will eventually finished it's job
// while constantly competing with other processes.
// In this case it competes with 30 normal jobs.
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
	
  // wolfietest();
  nicevaltest();
  prng_test();
  // lotterytest1();   // Basic priority test
  // lotterytest2();   // Dynamic test
  // lotterytest3();   // Starvation Test
  // policytest();
	exit();
}