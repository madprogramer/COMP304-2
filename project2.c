#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
/**
 * pthread_sleep takes an integer number of seconds to pause the current thread
 * original by Yingwu Zhu
 * updated by Muhammed Nufail Farooqi
 * updated by Fahrican Kosar
 */
int pthread_sleep(double seconds){
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    if(pthread_mutex_init(&mutex,NULL)){
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL)){
        return -1;
    }

    struct timeval tp;
    struct timespec timetoexpire;
    // When to expire is an absolute time, so get the current time and add
    // it to our delay time
    gettimeofday(&tp, NULL);
    long new_nsec = tp.tv_usec * 1000 + (seconds - (long)seconds) * 1e9;
    timetoexpire.tv_sec = tp.tv_sec + (long)seconds + (new_nsec / (long)1e9);
    timetoexpire.tv_nsec = new_nsec % (long)1e9;

    pthread_mutex_lock(&mutex);
    int res = pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);

    //Upon successful completion, a value of zero shall be returned
    return res;
}

//OUR CODE STARTS HERE

#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#define MAXCOMMENTATORS 400
#define CURARG 1+2*i
#define PROBABILITY_RESOLUTION 10000

pthread_mutex_t PANELMUTEX, BREAKINGMUTEX;
pthread_cond_t NEXTSPEAKER[MAXCOMMENTATORS], BREAKINGNEWS;
sem_t cTurn, answerDone, watchmanTurn,breakingDone;

//GLOBAL VARIABLES
int N,Q,time_int,debate_over,current_speaker;
double T,P,B;
time_t seed;

int pthread_sleep_controlled(double seconds){
    //MODIFIED PTHREAD_SLEEP WHICH CHECKS THE GLOBAL COND BREAKINGMUTEX
    pthread_mutex_t mutex;
    if(pthread_mutex_init(&mutex,NULL)){
        return -1;
    }
    struct timeval tp;
    struct timespec timetoexpire;
    // When to expire is an absolute time, so get the current time and add
    // it to our delay time
    gettimeofday(&tp, NULL);
    long new_nsec = tp.tv_usec * 1000 + (seconds - (long)seconds) * 1e9;
    timetoexpire.tv_sec = tp.tv_sec + (long)seconds + (new_nsec / (long)1e9);
    timetoexpire.tv_nsec = new_nsec % (long)1e9;

    pthread_mutex_lock(&mutex);
    int res = pthread_cond_timedwait(&BREAKINGNEWS, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);

    //Upon successful completion, a value of zero shall be returned
    return res;
}


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
pthread_t watchman;

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
    for (n=0;n<N;n++) 
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
      current_speaker=next;
      pthread_cond_signal(&NEXTSPEAKER[next]);
      pthread_mutex_unlock(&PANELMUTEX);

      //Wait on this thread
      sem_wait(&answerDone);

      //Wait for breaking news to end
      pthread_mutex_lock (&BREAKINGMUTEX);
      pthread_mutex_unlock (&BREAKINGMUTEX);
    }

  }

  //KILL ALL COMMENTATORS AND STEP DOWN
  debate_over=1;
  pthread_exit(NULL);

}
void commentate(void * arg) {

  int id = (int *)arg;
  int status;
  //sigset_t   set;

  do{

    //Wait if decided
    while( !debate_over && hasDecided(id));

    //Check if debate is over
    if(debate_over){
      pthread_exit(NULL);
    }

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
    //TALK FOR t_speak, interupt if breaking news reported
    status = pthread_sleep_controlled(t);

    //CHECK IF finished or not
    if(status==ETIMEDOUT)
    //if(status)
      logtime(),printf("Commentator #%d finished speaking\n",id);
    else
      logtime(),printf("Commentator #%d is cut short due to breaking news\n",id);
    sem_post(&answerDone);
    //FINISH SPEAKING AFTER t_speak
    
  }while(1);
} 

void watch(void * arg){
  do{

    sem_wait(&watchmanTurn);
    if(debate_over){
      pthread_exit(NULL);
    }

    logtime();printf("Breaking news!\n");
    //logtime();printf("Current Speaker: %d\n",current_speaker);
    //pthread_kill(commentator[current_speaker],SIGCONT);
    pthread_mutex_lock (&BREAKINGMUTEX);
    pthread_cond_signal(&BREAKINGNEWS);
    //Wait 5 seconds
    pthread_sleep(5);
    logtime(),printf("Breaking news ends\n");
    pthread_mutex_unlock (&BREAKINGMUTEX);
    sem_post(&breakingDone);

  }
  while(1);
}

int main(int argc, char *argv[]){

  //INIT
  gettimeofday(&start, NULL);
  N=0,Q=0,time_int=0,current_speaker=-1;
  T=0,P=0,B=0;
  seed=NULL;
  debate_over=0;

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

  if(pthread_mutex_init(&PANELMUTEX,NULL)){
    return -1;
  }
  if(pthread_mutex_init(&BREAKINGMUTEX,NULL)){
    return -1;
  }
  if(pthread_cond_init(&BREAKINGNEWS,NULL)){
    return -1;
  }

  //Note: Semaphore takes a count of N, but inited to 0
  if(sem_init(&cTurn, 0, 0)){
    return -1;
  }
  if(sem_init(&answerDone, 0, 0)){
    return -1;
  }
  if(sem_init(&watchmanTurn, 0, 0)){
    return -1;
  }
  if(sem_init(&breakingDone, 0, 0)){
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
  pthread_create(&watchman, NULL, watch, "Watchman");

  //Wait until debate over to generate breaking news)
  do{

    //Sleep for 1 second
    pthread_sleep(1); 

    if( (1.0*rand())/RAND_MAX < B ){
      sem_post(&watchmanTurn);
      sem_wait(&breakingDone);
    }

  }while(!debate_over );
  //Wake up the watchman when the show is over
  sem_post(&watchmanTurn);

  //Tie up loose ends
  pthread_join(watchman, NULL);
  for (n=0;n<N;n++){
      pthread_join(commentator[n], NULL);
      pthread_cond_destroy(&NEXTSPEAKER[n]);
  }
  pthread_join(moderator, NULL);

  //Destroy Globals
  sem_destroy(&breakingDone);
  sem_destroy(&watchmanTurn);
  sem_destroy(&cTurn);
  sem_destroy(&answerDone);
  pthread_mutex_destroy(&PANELMUTEX);
  pthread_mutex_destroy(&BREAKINGMUTEX);
  pthread_cond_destroy(&BREAKINGNEWS);

  return 0;
} 