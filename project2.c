
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#define NULL  __DARWIN_NULL
#define __DARWIN_NULL ((void *)0)
#define MAXCOMMENTATORS 400
#define CURARG 1+2*i
 /****************************************************************************** 
  pthread_sleep takes an integer number of seconds to pause the current thread 
  original by Yingwu Zhu
  updated by Muhammed Nufail Farooqi
  *****************************************************************************/

pthread_t moderator;
pthread_t commentator[MAXCOMMENTATORS];
/*pthread_mutex_t mutex;
pthread_cond_t conditionvar;
struct timespec timetoexpire;*/

int pthread_sleep (int seconds)
{
   pthread_mutex_t mutex;
   pthread_cond_t conditionvar;
   struct timespec timetoexpire;
   if(pthread_mutex_init(&mutex,NULL))
    {
      return -1;
    }
   if(pthread_cond_init(&conditionvar,NULL))
    {
      return -1;
    }
   struct timeval tp;
   //When to expire is an absolute time, so get the current time and add //it to our delay time
   gettimeofday(&tp, NULL);
   timetoexpire.tv_sec = tp.tv_sec + seconds; timetoexpire.tv_nsec = tp.tv_usec * 1000;

   pthread_mutex_lock (&mutex);
   int res =  pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
   pthread_mutex_unlock (&mutex);
   pthread_mutex_destroy(&mutex);
   pthread_cond_destroy(&conditionvar);

   //Upon successful completion, a value of zero shall be returned
   return res;

}

void moderate(void * arg) {
    printf("Moderator asks question %d\n",(int *)arg);
    //QUESTION ASKED SIGNAL!

    //Wait for answers
}
void commentate(void * arg) {
    //TODO: Position in Queue

    //WAIT
    //WAKE UP
    printf("Commentator #%d generates answer, position in queue: 0\n",(int *)arg);
    //DIE
} 
int main(int argc, char *argv[]){

  //Init
  int N=0,Q=0;
  double T=0,P=0;

  if( argc == 9 ) {

    //Parse Arguments
    int i=0;
    for(;i<4;i++){
      if(!strcmp(argv[CURARG], "-n"))
        sscanf(argv[CURARG+1], "%d", &N);
      if(!strcmp(argv[CURARG], "-q"))
        sscanf(argv[CURARG+1], "%d", &Q);
      if(!strcmp(argv[CURARG], "-t"))
        sscanf(argv[CURARG+1], "%lf", &T);
      if(!strcmp(argv[CURARG], "-p"))
        //printf("%s\n", argv[CURARG+1]);
        sscanf(argv[CURARG+1], "%lf", &P);
    }

  }
  else{
    printf("Error: Missing count of commentators!\n");
    return 1;
  }

  //Panel
  int n, q;
  for(q=1;q<=Q;q++) {

    pthread_create(&moderator, NULL, moderate, q);
    for (n=1;n<=N;n++){
      pthread_create(&commentator[n], NULL, commentate, n);
    }

    for (n=1;n<=N;n++){
      pthread_join(commentator[n], NULL);
    }
    pthread_join(moderator, NULL);
  }

  return 0;
} 