
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
//GLOBAL QUEUE
int QUEUE[MAXCOMMENTATORS];
//head refers to NEXT push, tail refers CURRENT pop
int head=0,tail=0,qsize=0;

int push(int tid){
  qsize++;
  QUEUE[head++] = tid;
  head%=MAXCOMMENTATORS;
  return qsize;
}

int pop(){
  qsize--;
  int top = QUEUE[tail++];
  tail%=MAXCOMMENTATORS;
  return top;
}
//END

//DETERMINE IF ALL COMMENTATORS HAVE DECIDED WHETHER OR NOT TO ANSWER
int decided[MAXCOMMENTATORS];

//returns 1 if all threads have decided, 0 otherwise
int everyoneDecided(){
  for (int i = 0; i < MAXCOMMENTATORS; ++i)
    if (!decided[i]) return 0;
  return 1;
}

void awaitDecisions(){
  for (int i = 0; i < MAXCOMMENTATORS; ++i)
    decided[i] = 0;
}

//TIME VALS
struct timeval start, current;

void logtime(){
  gettimeofday(&current, NULL);

  int min=(current.tv_sec-start.tv_sec)/60;
  int sec=(current.tv_sec-start.tv_sec)%60;
  int microsec=(current.tv_usec-start.tv_usec)/1000;

  printf("[%02d:%02d.%03d]\n",min,sec,microsec);
}

//PTHREADS
pthread_t moderator;
pthread_t commentator[MAXCOMMENTATORS];

void moderate(void * arg) {
    logtime();printf("Moderator asks question %d\n",(int *)arg);
    //QUESTION ASKED SIGNAL!

    //Wait for answers
}
void commentate(void * arg) {
    int id = (int *)arg;
    //Decide on whether or not to answer

    //TODO: LOCK QUEUE
    //Position in Queue
    int pos = push(id);
    logtime();printf("Commentator #%d generates answer, position in queue: %d\n",id,pos);
    //TODO: WAIT
    //TODO: WAKE UP
    //t_speak
    //DIE
} 

int main(int argc, char *argv[]){

  //Init
  int N=0,Q=0,time_int=0;
  double T=0,P=0,B=0;
  time_t seed=NULL;

  gettimeofday(&start, NULL);

  if( argc == 9 || argc == 11 || argc == 13) {

    //Parse Arguments
    int i=0, argcc = argc/2;
    for(;i<argcc;i++){
      if(!strcmp(argv[CURARG], "-n"))
        sscanf(argv[CURARG+1], "%d", &N);
      if(!strcmp(argv[CURARG], "-q"))
        sscanf(argv[CURARG+1], "%d", &Q);
      if(!strcmp(argv[CURARG], "-t"))
        sscanf(argv[CURARG+1], "%lf", &T);
      if(!strcmp(argv[CURARG], "-p"))
        sscanf(argv[CURARG+1], "%lf", &P);
      if(!strcmp(argv[CURARG], "-b"))
        sscanf(argv[CURARG+1], "%lf", &B);
      if(!strcmp(argv[CURARG], "-seed")){
        sscanf(argv[CURARG+1], "%d", &time_int);
        seed=time_int;
      }
    }

  srand(seed);

  }
  else{
    printf("Error: Missing count of commentators!\n");
    return 1;
  }

  //Panel
  int n, q;
  for(q=1;q<=Q;q++) {

    //Anticipate New Responses
    awaitDecisions();

    pthread_create(&moderator, NULL, moderate, q);
    for (n=1;n<=N;n++){
      pthread_create(&commentator[n], NULL, commentate, n);
    }

    //Wait for all threads to make a decision
    while(!everyoneDecided());

    for (n=1;n<=N;n++){
      pthread_join(commentator[n], NULL);
    }
    pthread_join(moderator, NULL);
  }

  return 0;
} 