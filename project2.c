
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <signal.h>
#define NULL  __DARWIN_NULL
#define __DARWIN_NULL ((void *)0)
#define MAXCOMMENTATORS 400
#define CURARG 1+2*i
#define PROBABILITY_RESOLUTION 10000
 /****************************************************************************** 
  pthread_sleep takes an integer number of seconds to pause the current thread 
  original by Yingwu Zhu
  updated by Muhammed Nufail Farooqi
  *****************************************************************************/
pthread_mutex_t PANELMUTEX;
pthread_cond_t NEXTSPEAKER[MAXCOMMENTATORS];
sem_t cTurn;

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

//GLOBAL VARIABLES
int N,Q,time_int;
double T,P,B;
time_t seed;

//GLOBAL QUEUE
//int QUEUE[MAXCOMMENTATORS];
pthread_t QUEUE[MAXCOMMENTATORS];
//head refers to NEXT push, tail refers CURRENT pop
int head=0,tail=0,qsize=0;

int push(int tid){
//int push(pthread_t tid){
  //qsize++;
  QUEUE[head++] = tid;
  head%=MAXCOMMENTATORS;
  return qsize++;
}

//pthread_t pop(){
int pop(){
  qsize--;
  //pthread_t top = QUEUE[tail++];
  int top = QUEUE[tail++];
  tail%=MAXCOMMENTATORS;
  return top;
}
//END

//DETERMINE IF ALL COMMENTATORS HAVE DECIDED WHETHER OR NOT TO ANSWER
int decided[MAXCOMMENTATORS];

//returns 1 if all threads have decided, 0 otherwise
int everyoneDecided(){
  for (int i = 0; i < N; ++i)
    if (!decided[i]) return 0;
  return 1;
}

void awaitDecisions(){
  for (int i = 0; i < N; ++i)
    decided[i] = 0;
}

void decide(int i){
  decided[i] = 1;
}

int hasDecided(int i){
  return decided[i];
}

//TIME VALS
struct timeval start, current;

void logtime(){
  gettimeofday(&current, NULL);

  int min=(current.tv_sec-start.tv_sec)/60;
  int sec=(current.tv_sec-start.tv_sec)%60;
  long long int microsec=(current.tv_usec-start.tv_usec)/1000;

  printf("[%02d:%02d.%03lld] ",min,sec,microsec);
}

//PTHREADS
pthread_t moderator;
pthread_t commentator[MAXCOMMENTATORS];

void moderate(void * arg) {
  
  //ignore arg for now, Q is global
  int n, q, next;
  pthread_t answerer;
  for(q=1;q<=Q;q++) {
    //Anticipate New Responses
    //sem_wait(&mTurn);
    logtime();printf("Moderator asks question %d\n",q);
    awaitDecisions();

    //Signal all commentator threads
    for (n=1;n<=N;n++) 
      sem_post(&cTurn);
    //Wait for all threads to make a decision
    while(!everyoneDecided());
    //Wait for queue to empty
    while(qsize!=0){
      //Pop Front of Queue and Signal it
      pthread_mutex_lock(&PANELMUTEX);
      //answerer = pop();
      //pthread_kill(answerer,SIGUSR1);

      next = pop();
      pthread_cond_signal(&NEXTSPEAKER[next]);

      pthread_mutex_unlock(&PANELMUTEX);
    }

  }

  //TODO: KILL ALL COMMENTATORS?

}
void commentate(void * arg) {

  int id = (int *)arg;
  //sigset_t   set;

  do{
    //Wait if decided
    while(hasDecided(id));
    //Commentator Turn
    sem_wait(&cTurn);
    int yes=0;

    //Decide on whether or not to answer
    //if( rand()%PROBABILITY_RESOLUTION < P*PROBABILITY_RESOLUTION ){
    if( (1.0*rand())/RAND_MAX < P ){
      yes=1;
    }
    decide(id);
    if (!yes) continue;
    /*if (!yes) {
      //signal(mTurn);
      continue;
    }*/

    //LOCK&UNLOCK QUEUE
    pthread_mutex_lock(&PANELMUTEX);
    //int pos = push(pthread_self());
    int pos = push(id);
    pthread_mutex_unlock(&PANELMUTEX);

    logtime();printf("Commentator #%d generates answer, position in queue: %d\n",id,pos);
    
    //WAIT FOR TURN TO SPEAK
    //sigemptyset(&set);
    pthread_mutex_lock (&PANELMUTEX);
    //WAKE UP
    pthread_cond_wait(&NEXTSPEAKER[id], &PANELMUTEX);
    pthread_mutex_unlock (&PANELMUTEX);
    /*if(sigaddset(&set, SIGUSR1) == -1) {
    perror("Sigaddset error");
    pthread_exit((void *)1);
    }

    if(sigwait(&set,(void *) 0) ) {
        perror("Sigwait error");
        pthread_exit((void *)2);
    } */


    double t = 1 + (rand() / (RAND_MAX / (T-1)));

    logtime();printf("Commentator #%d's turn to speak for %f seconds\n",id,t);
    //TODO: TALK FOR t_speak
    //TODO: FINISH SPEAKING AFTER t_speak
  }while(1);
} 

int main(int argc, char *argv[]){

  //INIT
  gettimeofday(&start, NULL);
  N=0,Q=0,time_int=0;
  T=0,P=0,B=0;
  seed=NULL;

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

  //SEED TIME
  srand(seed);
  }
  else
  {
    printf("Error: Missing count of commentators!\n");
    return 1;
  }

  if(pthread_mutex_init(&PANELMUTEX,NULL))
  {
    return -1;
  }
  //Note: Semaphore takes a count of N-1
  if(sem_init(&cTurn, 0, 0)){
    return -1;
  }
  //Set up the Q&A Panel
  int n;
  //awaitDecisions();
  //pthread_create(&moderator, NULL, moderate, Q);
  pthread_create(&moderator, NULL, moderate, "Moderator");
  for (n=0;n<N;n++){
    pthread_create(&commentator[n], NULL, commentate, n);
    if(pthread_cond_init(&NEXTSPEAKER[n],NULL))
    {
      return -1;
    }
  }

  //Tie up loose ends
  for (n=0;n<N;n++){
      pthread_join(commentator[n], NULL);
      pthread_cond_destroy(&NEXTSPEAKER[n]);
  }
  pthread_join(moderator, NULL);


  //Destroy Globals
  pthread_mutex_destroy(&PANELMUTEX);
  sem_destroy(&cTurn);

  return 0;
} 